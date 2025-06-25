#include "Generator.h"
#include "../../backend/runtime/HtmlOutput.h"
#include "../../backend/runtime/PcapRuntime.h"
#include "../../backend/runtime/ProtocolExtractors.h"
#include "../../backend/domain-specific/Calculator.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* MODULE INTERNAL STATE */

const char _indentationCharacter = ' ';
const char _indentationSize = 4;
static Logger * _logger = NULL;
static int _packetCount = 0;
static int _httpErrors = 0;
static int _tcpConnections = 0;
static int _udpPackets = 0;

// Estructura para almacenar configuración de captura/import
typedef struct {
	char filename[256];
	char filter[256];
	int is_import; // 1 si es import, 0 si es capture
} CaptureConfig;

// Estructura para almacenar los datos extraídos de cada paquete
typedef struct PacketInfo {
	char src_ip[64];
	char dst_ip[64];
	int src_port;
	int dst_port;
	int protocol; // 0: TCP, 1: UDP, 2: HTTP
	int http_status;
	char http_method[16];
	int http_response_code;
	double timestamp;
	struct PacketInfo* next;
} PacketInfo;

static PacketInfo* packet_list_head = NULL;
static PacketInfo* packet_list_tail = NULL;

// Variables globales para el procesamiento de queries
static char* current_pcap_file = NULL;
static char* current_filter = NULL;
static FieldList* current_extract_fields = NULL;
static Condition* current_where_condition = NULL;
static FieldList* current_group_fields = NULL;
static Condition* current_having_condition = NULL;
static Expression* current_aggregation = NULL;

void initializeGeneratorModule() {
	_logger = createLogger("Generator");
}

// Forward declarations
static void free_packet_list(void);
static void add_packet_info(PacketInfo* info);

void shutdownGeneratorModule() {
	if (_logger != NULL) {
		destroyLogger(_logger);
	}
	// Limpiar lista de paquetes
	free_packet_list();
}

/** PRIVATE FUNCTIONS */

static void _generateHtmlHeader(void);
static void _generateHtmlFooter(void);
static void _generateMetrics(void);
static void _generateTables(void);
static void _processProgram(Program * program);
static void _processStatement(Statement * statement);
static void _processCaptureStatement(CaptureStatement * capture);
static void _processExtractStatement(ExtractStatement * extract);
static void _processFilterStatement(FilterStatement * filter);
static void _processAlertStatement(AlertStatement * alert);
static void _processGroupStatement(GroupStatement * group);
static void _processAggregation(Expression * expression);
static char * _indentation(const unsigned int level);
static void _output(const unsigned int indentationLevel, const char * const format, ...);
static bool find_capture_or_import(Program* program, CaptureConfig* config);
static void process_import_statement(ImportExportStatement* import);
static void process_variable_declaration(VariableDeclaration* var_decl);
static bool evaluate_condition(Condition* condition, PacketInfo* packet);
static bool evaluate_expression(Expression* expr, PacketInfo* packet, double* result);
static bool evaluate_factor(Factor* factor, PacketInfo* packet, double* result);
static bool evaluate_field(Field* field, PacketInfo* packet, double* result);
static void process_group_by_results(void);

// Helper to read DSL file contents
static char* read_dsl_file(const char* filename) {
	FILE* f = fopen(filename, "r");
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);
	char* buf = (char*)malloc(len + 1);
	if (!buf) { fclose(f); return NULL; }
	fread(buf, 1, len, f);
	buf[len] = '\0';
	fclose(f);
	return buf;
}

// Función para agregar un paquete a la lista
static void add_packet_info(PacketInfo* info) {
	if (!packet_list_head) {
		packet_list_head = info;
		packet_list_tail = info;
	} else {
		packet_list_tail->next = info;
		packet_list_tail = info;
	}
	info->next = NULL;
}

// Handler real para procesar paquetes y actualizar métricas y lista
static void real_packet_handler(u_char* user, const struct pcap_pkthdr* header, const u_char* packet) {
	logDebugging(_logger, "Entered real_packet_handler");
	(void)user;
	
	if (!header || !packet) {
		logError(_logger, "real_packet_handler: header or packet is NULL");
		return;
	}
	
	PacketInfo* info = (PacketInfo*)calloc(1, sizeof(PacketInfo));
	if (!info) {
		logError(_logger, "Failed to allocate PacketInfo");
		return;
	}
	
	// Initialize all fields to safe defaults
	memset(info->src_ip, 0, sizeof(info->src_ip));
	memset(info->dst_ip, 0, sizeof(info->dst_ip));
	info->src_port = 0;
	info->dst_port = 0;
	info->protocol = -1;
	info->http_status = 0;
	memset(info->http_method, 0, sizeof(info->http_method));
	info->http_response_code = 0;
	info->timestamp = 0.0;
	info->next = NULL;
	
	// Extraer timestamp
	info->timestamp = (double)header->ts.tv_sec + (double)header->ts.tv_usec / 1000000.0;
	logDebugging(_logger, "Extracted timestamp: %f", info->timestamp);
	
	// Extraer campos IP
	if (extract_ip_src_addr(packet, info->src_ip) != 0) {
		logWarning(_logger, "Failed to extract IP source address");
		strcpy(info->src_ip, "0.0.0.0");
	}
	if (extract_ip_dst_addr(packet, info->dst_ip) != 0) {
		logWarning(_logger, "Failed to extract IP destination address");
		strcpy(info->dst_ip, "0.0.0.0");
	}
	logDebugging(_logger, "Extracted src_ip: %s, dst_ip: %s", info->src_ip, info->dst_ip);
	
	// TCP
	if (is_tcp_packet(packet)) {
		if (extract_tcp_src_port(packet, &info->src_port) != 0) {
			logWarning(_logger, "Failed to extract TCP source port");
			info->src_port = 0;
		}
		if (extract_tcp_dst_port(packet, &info->dst_port) != 0) {
			logWarning(_logger, "Failed to extract TCP destination port");
			info->dst_port = 0;
		}
		info->protocol = 0;
		_tcpConnections++;
		logDebugging(_logger, "TCP packet: src_port=%d, dst_port=%d", info->src_port, info->dst_port);
	}
	// UDP
	else if (is_udp_packet(packet)) {
		if (extract_udp_src_port(packet, &info->src_port) != 0) {
			logWarning(_logger, "Failed to extract UDP source port");
			info->src_port = 0;
		}
		if (extract_udp_dst_port(packet, &info->dst_port) != 0) {
			logWarning(_logger, "Failed to extract UDP destination port");
			info->dst_port = 0;
		}
		info->protocol = 1;
		_udpPackets++;
		logDebugging(_logger, "UDP packet: src_port=%d, dst_port=%d", info->src_port, info->dst_port);
	}
	
	// HTTP
	if (is_http_packet(packet)) {
		if (extract_http_status_code(packet, &info->http_status) != 0) {
			logWarning(_logger, "Failed to extract HTTP status code");
			info->http_status = 0;
		}
		if (extract_http_method(packet, info->http_method) != 0) {
			logWarning(_logger, "Failed to extract HTTP method");
			strcpy(info->http_method, "UNKNOWN");
		}
		info->protocol = 2;
		info->http_response_code = info->http_status;
		if (info->http_status >= 400) {
			_httpErrors++;
		}
		logDebugging(_logger, "HTTP packet: status=%d, method=%s", info->http_status, info->http_method);
	}
	
	_packetCount++;
	logDebugging(_logger, "Packet processed. _packetCount=%d", _packetCount);
	
	// Aplicar filtro WHERE si existe
	if (current_where_condition) {
		if (!evaluate_condition(current_where_condition, info)) {
			logDebugging(_logger, "Packet did not match WHERE condition");
			free(info);
			return;
		}
	}
	
	add_packet_info(info);
	logDebugging(_logger, "Packet added to list");
}

// Limpia la lista de paquetes
static void free_packet_list() {
	PacketInfo* current = packet_list_head;
	while (current) {
		PacketInfo* next = current->next;
		free(current);
		current = next;
	}
	packet_list_head = NULL;
	packet_list_tail = NULL;
}

// Busca declaraciones de import o capture en el programa
static bool find_capture_or_import(Program* program, CaptureConfig* config) {
	logDebugging(_logger, "find_capture_or_import: entering function");
	
	if (!program || !config) {
		logError(_logger, "find_capture_or_import: program o config es NULL");
		return false;
	}
	
	logDebugging(_logger, "find_capture_or_import: program=%p, config=%p", program, config);
	logDebugging(_logger, "find_capture_or_import: program->statements=%p", program->statements);
	
	StatementList* current = program->statements;
	logDebugging(_logger, "find_capture_or_import: current=%p", current);
	
	while (current) {
		logDebugging(_logger, "find_capture_or_import: processing statement list item, current=%p", current);
		
		if (current->statement) {
			logDebugging(_logger, "find_capture_or_import: statement=%p, type=%d", current->statement, current->statement->type);
			
			if (current->statement->type == IMPORT_STATEMENT) {
				logDebugging(_logger, "find_capture_or_import: found IMPORT_STATEMENT");
				if (current->statement->import_export.filename) {
					logInformation(_logger, "find_capture_or_import: found import statement for %s", 
						current->statement->import_export.filename);
					strncpy(config->filename, current->statement->import_export.filename, sizeof(config->filename) - 1);
					config->filename[sizeof(config->filename) - 1] = '\0'; // Ensure null termination
					config->is_import = 1;
					logDebugging(_logger, "find_capture_or_import: returning true for import");
					return true;
				} else {
					logError(_logger, "find_capture_or_import: import statement has NULL filename");
				}
			} else if (current->statement->type == CAPTURE_STATEMENT) {
				logDebugging(_logger, "find_capture_or_import: found CAPTURE_STATEMENT");
				if (current->statement->capture.interface) {
					logInformation(_logger, "find_capture_or_import: found capture statement for %s", 
						current->statement->capture.interface);
					strncpy(config->filename, current->statement->capture.interface, sizeof(config->filename) - 1);
					config->filename[sizeof(config->filename) - 1] = '\0'; // Ensure null termination
					config->is_import = 0;
					logDebugging(_logger, "find_capture_or_import: returning true for capture");
					return true;
				} else {
					logError(_logger, "find_capture_or_import: capture statement has NULL interface");
				}
			}
		} else {
			logWarning(_logger, "find_capture_or_import: statement is NULL in list");
		}
		current = current->next;
		logDebugging(_logger, "find_capture_or_import: moving to next statement, current=%p", current);
	}
	
	logDebugging(_logger, "find_capture_or_import: no import or capture statements found");
	return false;
}

// Procesa declaraciones de import
static void process_import_statement(ImportExportStatement* import) {
	if (!import) return;
	
	logInformation(_logger, "Processing import from: %s", import->filename);
	current_pcap_file = strdup(import->filename);
}

// Procesa declaraciones de variables
static void process_variable_declaration(VariableDeclaration* var_decl) {
	if (!var_decl) return;
	
	logInformation(_logger, "Processing variable declaration: %s", var_decl->identifier);
	// TODO: Implementar almacenamiento de variables
}

// Evalúa una condición contra un paquete
static bool evaluate_condition(Condition* condition, PacketInfo* packet) {
	if (!condition || !packet) {
		logError(_logger, "evaluate_condition: condition o packet es NULL");
		return false;
	}
	logDebugging(_logger, "evaluate_condition: type=%d", condition->type);
	switch (condition->type) {
		case COMPARISON: {
			double left_val, right_val;
			if (evaluate_expression(condition->comparison.left, packet, &left_val) &&
				evaluate_expression(condition->comparison.right, packet, &right_val)) {
				switch (condition->comparison.op) {
					case EQUALS_OP: return left_val == right_val;
					case NOT_EQUALS_OP: return left_val != right_val;
					case LESS_THAN_OP: return left_val < right_val;
					case GREATER_THAN_OP: return left_val > right_val;
					case GREATER_THAN_OR_EQUALS_OP: return left_val >= right_val;
					case LESS_THAN_OR_EQUALS_OP: return left_val <= right_val;
					default: return false;
				}
			}
			logWarning(_logger, "evaluate_condition: No se pudo evaluar una comparación");
			return false;
		}
		case LOGICAL_AND:
			return evaluate_condition(condition->logical.left, packet) &&
				   evaluate_condition(condition->logical.right, packet);
		case LOGICAL_OR:
			return evaluate_condition(condition->logical.left, packet) ||
				   evaluate_condition(condition->logical.right, packet);
		default:
			logWarning(_logger, "evaluate_condition: Tipo de condición no soportado: %d", condition->type);
			return false;
	}
}

// Evalúa una expresión contra un paquete
static bool evaluate_expression(Expression* expr, PacketInfo* packet, double* result) {
	if (!expr || !packet || !result) {
		logError(_logger, "evaluate_expression: expr, packet o result es NULL");
		return false;
	}
	logDebugging(_logger, "evaluate_expression: type=%d", expr->type);
	switch (expr->type) {
		case FACTOR:
			return evaluate_factor(expr->factor, packet, result);
		case ADDITION: {
			double left_val, right_val;
			if (evaluate_expression(expr->leftExpression, packet, &left_val) &&
				evaluate_expression(expr->rightExpression, packet, &right_val)) {
				*result = left_val + right_val;
				return true;
			}
			logWarning(_logger, "evaluate_expression: No se pudo evaluar suma");
			return false;
		}
		case SUBTRACTION: {
			double left_val, right_val;
			if (evaluate_expression(expr->leftExpression, packet, &left_val) &&
				evaluate_expression(expr->rightExpression, packet, &right_val)) {
				*result = left_val - right_val;
				return true;
			}
			logWarning(_logger, "evaluate_expression: No se pudo evaluar resta");
			return false;
		}
		case MULTIPLICATION: {
			double left_val, right_val;
			if (evaluate_expression(expr->leftExpression, packet, &left_val) &&
				evaluate_expression(expr->rightExpression, packet, &right_val)) {
				*result = left_val * right_val;
				return true;
			}
			logWarning(_logger, "evaluate_expression: No se pudo evaluar multiplicación");
			return false;
		}
		case DIVISION: {
			double left_val, right_val;
			if (evaluate_expression(expr->leftExpression, packet, &left_val) &&
				evaluate_expression(expr->rightExpression, packet, &right_val)) {
				if (right_val != 0) {
					*result = left_val / right_val;
					return true;
				}
			}
			logWarning(_logger, "evaluate_expression: No se pudo evaluar división");
			return false;
		}
		default:
			logWarning(_logger, "evaluate_expression: Tipo de expresión no soportado: %d", expr->type);
			return false;
	}
}

// Evalúa un factor contra un paquete
static bool evaluate_factor(Factor* factor, PacketInfo* packet, double* result) {
	if (!factor || !packet || !result) return false;
	
	switch (factor->type) {
		case CONSTANT:
			if (factor->constant && factor->constant->value.type == INTEGER_TYPE) {
				*result = (double)factor->constant->value.integer;
				return true;
			}
			return false;
		case FIELD:
			return evaluate_field(factor->field, packet, result);
		case VARIABLE_REFERENCE:
			// TODO: Implementar referencia a variables
			return false;
		default:
			return false;
	}
}

// Evalúa un campo contra un paquete
static bool evaluate_field(Field* field, PacketInfo* packet, double* result) {
	if (!field || !packet || !result) {
		logError(_logger, "evaluate_field: field, packet o result es NULL");
		return false;
	}
	if (!field->protocol || !field->name) {
		logError(_logger, "evaluate_field: field->protocol o field->name es NULL");
		return false;
	}
	logDebugging(_logger, "evaluate_field: protocol='%s', name='%s'", field->protocol, field->name);

	if (strcmp(field->protocol, "ip") == 0) {
		if (strcmp(field->name, "src_addr") == 0) {
			// Para comparaciones de IP, usar hash simple
			*result = (double)(packet->src_ip[0] + packet->src_ip[1] + packet->src_ip[2] + packet->src_ip[3]);
			return true;
		}
	} else if (strcmp(field->protocol, "http") == 0) {
		if (strcmp(field->name, "response_code") == 0) {
			*result = (double)packet->http_response_code;
			return true;
		}
	} else if (strcmp(field->protocol, "tcp") == 0) {
		if (strcmp(field->name, "src_port") == 0) {
			*result = (double)packet->src_port;
			return true;
		} else if (strcmp(field->name, "dst_port") == 0) {
			*result = (double)packet->dst_port;
			return true;
		}
	}
	logWarning(_logger, "evaluate_field: Campo no soportado protocol='%s', name='%s'", field->protocol, field->name);
	return false;
}

// Procesa los resultados agrupados
static void process_group_by_results(void) {
	logDebugging(_logger, "process_group_by_results: inicio");
	if (!current_group_fields) {
		logWarning(_logger, "process_group_by_results: current_group_fields es NULL");
		return;
	}
	if (!packet_list_head) {
		logWarning(_logger, "process_group_by_results: packet_list_head es NULL (no hay paquetes)");
		return;
	}
	// Build group-by results table (ip.src_addr, count)
	const char* headers[] = {"ip.src_addr", "Count"};
	// Count groups
	int group_count = 0;
	struct { char ip[64]; int count; } groups[128];
	memset(groups, 0, sizeof(groups));
	// Simple grouping logic (for demonstration)
	PacketInfo* pkt = packet_list_head;
	while (pkt) {
		int found = 0;
		for (int i = 0; i < group_count; ++i) {
			if (strcmp(groups[i].ip, pkt->src_ip) == 0) {
				groups[i].count++;
				found = 1;
				break;
			}
		}
		if (!found && group_count < 128) {
			strncpy(groups[group_count].ip, pkt->src_ip, 63);
			groups[group_count].ip[63] = '\0';
			groups[group_count].count = 1;
			group_count++;
		}
		pkt = pkt->next;
	}
	// Build rows for the table
	const char*** rows = malloc(group_count * sizeof(char**));
	for (int i = 0; i < group_count; ++i) {
		char** row = malloc(2 * sizeof(char*));
		row[0] = strdup(groups[i].ip);
		char* count_str = malloc(16);
		snprintf(count_str, 16, "%d", groups[i].count);
		row[1] = count_str;
		rows[i] = (const char**)row;
	}
	html_add_simple_table(headers, 2, rows, group_count);
	// Free rows
	for (int i = 0; i < group_count; ++i) {
		free((void*)rows[i][0]);
		free((void*)rows[i][1]);
		free((void*)rows[i]);
	}
	free(rows);
}

/**
 * Generates HTML header with Bootstrap and Chart.js
 */
static void _generateHtmlHeader(void) {
	_output(0, "%s",
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"    <meta charset=\"UTF-8\">\n"
		"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		"    <title>Verm Network Analysis Report</title>\n"
		"    <link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n"
		"    <script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>\n"
		"    <style>\n"
		"        .metric-card { transition: transform 0.2s; }\n"
		"        .metric-card:hover { transform: translateY(-2px); }\n"
		"        .chart-container { position: relative; height: 400px; margin: 20px 0; }\n"
		"    </style>\n"
		"</head>\n"
		"<body class=\"bg-light\">\n"
		"    <div class=\"container-fluid\">\n"
		"        <div class=\"row\">\n"
		"            <div class=\"col-12\">\n"
		"                <h1 class=\"text-center my-4\">Verm Network Analysis Report</h1>\n"
		"                <div class=\"row\" id=\"metrics-container\">\n"
	);
}

/**
 * Generates HTML footer
 */
static void _generateHtmlFooter(void) {
	_output(0, "%s",
		"            </div>\n"
		"        </div>\n"
		"    </div>\n"
		"    <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.bundle.min.js\"></script>\n"
		"</body>\n"
		"</html>\n"
	);
}

/**
 * Generates metrics cards
 */
static void _generateMetrics(void) {
	_output(0, "%s",
		"                    <div class=\"col-md-3 col-sm-6 mb-3\">\n"
		"                        <div class=\"card metric-card bg-primary text-white\">\n"
		"                            <div class=\"card-body text-center\">\n"
		"                                <h5 class=\"card-title\">Total Packets</h5>\n"
		"                                <h2 class=\"card-text\">%d</h2>\n"
		"                                <p class=\"card-text small\">Processed packets</p>\n"
		"                            </div>\n"
		"                        </div>\n"
		"                    </div>\n", _packetCount
	);
	
	_output(0, "%s",
		"                    <div class=\"col-md-3 col-sm-6 mb-3\">\n"
		"                        <div class=\"card metric-card bg-danger text-white\">\n"
		"                            <div class=\"card-body text-center\">\n"
		"                                <h5 class=\"card-title\">HTTP Errors</h5>\n"
		"                                <h2 class=\"card-text\">%d</h2>\n"
		"                                <p class=\"card-text small\">HTTP error responses</p>\n"
		"                            </div>\n"
		"                        </div>\n"
		"                    </div>\n", _httpErrors
	);
	
	_output(0, "%s",
		"                    <div class=\"col-md-3 col-sm-6 mb-3\">\n"
		"                        <div class=\"card metric-card bg-success text-white\">\n"
		"                            <div class=\"card-body text-center\">\n"
		"                                <h5 class=\"card-title\">TCP Connections</h5>\n"
		"                                <h2 class=\"card-text\">%d</h2>\n"
		"                                <p class=\"card-text small\">TCP connections detected</p>\n"
		"                            </div>\n"
		"                        </div>\n"
		"                    </div>\n", _tcpConnections
	);
	
	_output(0, "%s",
		"                    <div class=\"col-md-3 col-sm-6 mb-3\">\n"
		"                        <div class=\"card metric-card bg-warning text-dark\">\n"
		"                            <div class=\"card-body text-center\">\n"
		"                                <h5 class=\"card-title\">UDP Packets</h5>\n"
		"                                <h2 class=\"card-text\">%d</h2>\n"
		"                                <p class=\"card-text small\">UDP packets detected</p>\n"
		"                            </div>\n"
		"                        </div>\n"
		"                    </div>\n", _udpPackets
	);
}

/**
 * Generates data tables
 */
static void _generateTables(void) {
	_output(0, "%s",
		"                </div>\n"
		"                <div class=\"row mt-4\">\n"
		"                    <div class=\"col-12\">\n"
		"                        <div class=\"card\">\n"
		"                            <div class=\"card-header\">\n"
		"                                <h5 class=\"mb-0\">Analysis Summary</h5>\n"
		"                            </div>\n"
		"                            <div class=\"card-body\">\n"
		"                                <div class=\"table-responsive\">\n"
		"                                    <table class=\"table table-striped table-hover\">\n"
		"                                        <thead class=\"table-dark\">\n"
		"                                            <tr>\n"
		"                                                <th>Metric</th>\n"
		"                                                <th>Value</th>\n"
		"                                                <th>Description</th>\n"
		"                                            </tr>\n"
		"                                        </thead>\n"
		"                                        <tbody>\n"
		"                                            <tr>\n"
		"                                                <td>Total Packets</td>\n"
		"                                                <td>%d</td>\n"
		"                                                <td>Number of packets processed</td>\n"
		"                                            </tr>\n"
		"                                            <tr>\n"
		"                                                <td>HTTP Errors</td>\n"
		"                                                <td>%d</td>\n"
		"                                                <td>HTTP responses with error codes</td>\n"
		"                                            </tr>\n"
		"                                            <tr>\n"
		"                                                <td>TCP Connections</td>\n"
		"                                                <td>%d</td>\n"
		"                                                <td>TCP connections detected</td>\n"
		"                                            </tr>\n"
		"                                            <tr>\n"
		"                                                <td>UDP Packets</td>\n"
		"                                                <td>%d</td>\n"
		"                                                <td>UDP packets detected</td>\n"
		"                                            </tr>\n"
		"                                        </tbody>\n"
		"                                    </table>\n"
		"                                </div>\n"
		"                            </div>\n"
		"                        </div>\n"
		"                    </div>\n"
		"                </div>\n", _packetCount, _httpErrors, _tcpConnections, _udpPackets
	);
}

/**
 * Processes the program AST
 */
static void _processProgram(Program * program) {
	if (!program) return;
	
	switch (program->type) {
		case PROGRAM_STATEMENTS:
			if (program->statements) {
				StatementList* current = program->statements;
				while (current) {
					_processStatement(current->statement);
					current = current->next;
				}
			}
			break;
		case PROGRAM_EXPRESSION:
			_processAggregation(program->expression);
			break;
		default:
			logError(_logger, "Unknown program type: %d", program->type);
			break;
	}
}

/**
 * Processes individual statements
 */
static void _processStatement(Statement * statement) {
	if (!statement) return;
	
	switch (statement->type) {
		case CAPTURE_STATEMENT:
			_processCaptureStatement(&statement->capture);
			break;
		case EXTRACT_STATEMENT:
			_processExtractStatement(&statement->extract);
			break;
		case FILTER_STATEMENT:
			_processFilterStatement(&statement->filter);
			break;
		case ALERT_STATEMENT:
			_processAlertStatement(&statement->alert);
			break;
		case IMPORT_STATEMENT:
			process_import_statement(&statement->import_export);
			break;
		case EXPORT_STATEMENT:
			// Process export statement
			logInformation(_logger, "Processing export to: %s", statement->import_export.filename);
			break;
		case VARIABLE_DECLARATION_STATEMENT:
			process_variable_declaration(&statement->variable_declaration);
			break;
		default:
			logWarning(_logger, "Unsupported statement type: %d", statement->type);
			break;
	}
	
	// Process group statement if present (it's a separate field, not a statement type)
	if (statement->group.group_fields) {
		_processGroupStatement(&statement->group);
	}
}

/**
 * Processes capture statements
 */
static void _processCaptureStatement(CaptureStatement * capture) {
	if (!capture) return;
	
	logInformation(_logger, "Processing capture from: %s", capture->interface);
	current_pcap_file = strdup(capture->interface);
	
	// Configurar filtro si existe
	if (capture->filter) {
		current_filter = strdup(""); // TODO: Convertir condición a string BPF
	}
}

/**
 * Processes extract statements
 */
static void _processExtractStatement(ExtractStatement * extract) {
	if (!extract) return;
	
	logInformation(_logger, "Processing extract statement");
	current_extract_fields = extract->fields;
	
	// Configurar condición WHERE si existe
	if (extract->filter) {
		current_where_condition = extract->filter;
	}
	
	// Process field list
	FieldList* current = extract->fields;
	while (current) {
		if (current->field) {
			logInformation(_logger, "Extracting field: %s.%s", 
				current->field->protocol, current->field->name);
		}
		current = current->next;
	}
}

/**
 * Processes filter statements
 */
static void _processFilterStatement(FilterStatement * filter) {
	if (!filter) return;
	
	logInformation(_logger, "Processing filter statement");
	current_where_condition = filter->condition;
	
	// Process filter condition
	if (filter->condition) {
		logInformation(_logger, "Filter condition detected");
	}
}

/**
 * Processes alert statements
 */
static void _processAlertStatement(AlertStatement * alert) {
	if (!alert) return;
	
	logInformation(_logger, "Processing alert: %s", alert->message);
	
	// Generate alert in HTML
	_output(0, "%s",
		"                <div class=\"alert alert-warning alert-dismissible fade show\" role=\"alert\">\n"
		"                    <strong>Alert:</strong> %s\n"
		"                    <button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"alert\"></button>\n"
		"                </div>\n", alert->message ? alert->message : "Alert triggered"
	);
}

/**
 * Processes group statements
 */
static void _processGroupStatement(GroupStatement * group) {
	if (!group) return;
	
	logInformation(_logger, "Processing group statement");
	current_group_fields = group->group_fields;
	current_having_condition = group->having;
	
	// Process group fields
	FieldList* current = group->group_fields;
	while (current) {
		if (current->field) {
			logInformation(_logger, "Grouping by: %s.%s", 
				current->field->protocol, current->field->name);
		}
		current = current->next;
	}
}

/**
 * Processes aggregation expressions
 */
static void _processAggregation(Expression * expression) {
	if (!expression) return;
	
	current_aggregation = expression;
	
	switch (expression->type) {
		case AGGREGATION_COUNT:
			logInformation(_logger, "Processing count aggregation");
			break;
		case AGGREGATION_SUM:
			logInformation(_logger, "Processing sum aggregation");
			break;
		case AGGREGATION_AVG:
			logInformation(_logger, "Processing average aggregation");
			break;
		case AGGREGATION_MIN:
			logInformation(_logger, "Processing min aggregation");
			break;
		case AGGREGATION_MAX:
			logInformation(_logger, "Processing max aggregation");
			break;
		default:
			logWarning(_logger, "Unsupported aggregation type: %d", expression->type);
			break;
	}
}

/**
 * Generates an indentation string for the specified level.
 */
static char * _indentation(const unsigned int level) {
	return indentation(_indentationCharacter, level, _indentationSize);
}

/**
 * Outputs a formatted string to stdout.
 */
static void _output(const unsigned int indentationLevel, const char * const format, ...) {
	va_list arguments;
	va_start(arguments, format);
	char * indentation = _indentation(indentationLevel);
	char * effectiveFormat = concatenate(2, indentation, format);
	
	vfprintf(stdout, effectiveFormat, arguments);
	fflush(stdout);
	
	free(effectiveFormat);
	free(indentation);
	va_end(arguments);
}

/** PUBLIC FUNCTIONS */

void generate(CompilerState * compilerState, const char* dsl_filename) {
	printf("DEBUG: Entering generate function\n");
	
	logDebugging(_logger, "Generating HTML output...");

	// Check if compilerState is valid
	if (!compilerState) {
		printf("DEBUG: compilerState is NULL\n");
		logError(_logger, "compilerState is NULL");
		return;
	}
	
	if (!compilerState->abstractSyntaxtTree) {
		printf("DEBUG: abstractSyntaxtTree is NULL\n");
		logError(_logger, "abstractSyntaxtTree is NULL");
		return;
	}
	
	Program* program = (Program*)compilerState->abstractSyntaxtTree;
	printf("DEBUG: program=%p\n", program);
	logDebugging(_logger, "compilerState->abstractSyntaxtTree=%p", compilerState->abstractSyntaxtTree);
	logDebugging(_logger, "program->statements=%p", program->statements);

	// Initialize HTML dashboard properly
	printf("DEBUG: About to initialize HTML dashboard\n");
	logDebugging(_logger, "About to initialize HTML dashboard...");
	if (html_init_dashboard("output.html", "Verm Network Analysis Report") != 0) {
		printf("DEBUG: Failed to initialize HTML dashboard\n");
		logError(_logger, "Failed to initialize HTML dashboard");
		return;
	}
	printf("DEBUG: HTML dashboard initialized successfully\n");
	logDebugging(_logger, "HTML dashboard initialized successfully");

	// Read DSL file and include in HTML
	if (dsl_filename) {
		printf("DEBUG: Reading DSL file: %s\n", dsl_filename);
		logDebugging(_logger, "Reading DSL file: %s", dsl_filename);
		char* dsl = read_dsl_file(dsl_filename);
		if (dsl) {
			html_add_query_section(dsl);
			logDebugging(_logger, "Added query section to HTML");
			free(dsl);
		}
	}

	// Inicializar métricas
	_packetCount = 0;
	_httpErrors = 0;
	_tcpConnections = 0;
	_udpPackets = 0;

	// Buscar import/capture y procesar tráfico real
	CaptureConfig config;
	memset(&config, 0, sizeof(config));
	printf("DEBUG: Searching for import/capture statements\n");
	logDebugging(_logger, "Searching for import/capture statements...");
	if (find_capture_or_import(program, &config)) {
		printf("DEBUG: Found import/capture: %s\n", config.filename);
		logInformation(_logger, "Found %s: %s", config.is_import ? "import" : "capture", config.filename);
		logDebugging(_logger, "Trying to open PCAP file: %s", config.filename);
		FILE* test_file = fopen(config.filename, "rb");
		if (!test_file) {
			printf("DEBUG: PCAP file does not exist: %s\n", config.filename);
			logError(_logger, "PCAP file does not exist or cannot be opened: %s", config.filename);
		} else {
			fclose(test_file);
			printf("DEBUG: PCAP file exists, initializing context\n");
			logDebugging(_logger, "PCAP file exists, initializing context...");
			PcapContext ctx;
			if (init_pcap_context(&ctx, config.filename, config.filter) == 0) {
				printf("DEBUG: PCAP context initialized, processing packets\n");
				logDebugging(_logger, "PCAP context initialized, about to process packets");
				process_packets(&ctx, real_packet_handler);
				printf("DEBUG: Finished processing packets\n");
				logDebugging(_logger, "Finished processing packets");
				cleanup_pcap_context(&ctx);
				logDebugging(_logger, "Cleaned up PCAP context");
			} else {
				printf("DEBUG: Failed to initialize PCAP context\n");
				logError(_logger, "Failed to initialize PCAP context for %s", config.filename);
			}
		}
	} else {
		printf("DEBUG: No import/capture found\n");
		logWarning(_logger, "No import nor capture found in program. No real traffic will be processed.");
	}
	printf("DEBUG: After packet processing\n");
	logDebugging(_logger, "After packet processing");

	printf("DEBUG: Building group-by table. packet_list_head=%p\n", packet_list_head);
	logDebugging(_logger, "Building group-by table. packet_list_head=%p", packet_list_head);
	if (!packet_list_head) {
		printf("DEBUG: No packets to process\n");
		logWarning(_logger, "No packets to process for group-by table.");
		html_add_section("Results", "<p>No packets found in the PCAP file.</p>");
		html_close_dashboard();
		return;
	}
	
	// Build group-by results table (ip.src_addr, count)
	printf("DEBUG: Starting to build group-by table\n");
	logDebugging(_logger, "Starting to build group-by table...");
	const char* headers[] = {"ip.src_addr", "Count"};
	int group_count = 0;
	struct { char ip[64]; int count; } groups[128];
	memset(groups, 0, sizeof(groups));
	PacketInfo* pkt = packet_list_head;
	while (pkt) {
		int found = 0;
		for (int i = 0; i < group_count; ++i) {
			if (strcmp(groups[i].ip, pkt->src_ip) == 0) {
				groups[i].count++;
				found = 1;
				break;
			}
		}
		if (!found && group_count < 128) {
			strncpy(groups[group_count].ip, pkt->src_ip, 63);
			groups[group_count].ip[63] = '\0';
			groups[group_count].count = 1;
			group_count++;
		}
		pkt = pkt->next;
	}
	printf("DEBUG: Group count: %d\n", group_count);
	logDebugging(_logger, "Group count: %d", group_count);
	
	if (group_count == 0) {
		printf("DEBUG: No groups found\n");
		logWarning(_logger, "No groups found for group-by table.");
		html_add_section("Results", "<p>No group-by results to display.</p>");
		html_close_dashboard();
		return;
	}
	
	printf("DEBUG: Allocating memory for rows\n");
	logDebugging(_logger, "Allocating memory for rows...");
	const char*** rows = malloc(group_count * sizeof(char**));
	if (!rows) {
		printf("DEBUG: Failed to allocate memory for rows\n");
		logError(_logger, "Failed to allocate memory for rows");
		html_close_dashboard();
		return;
	}
	
	printf("DEBUG: Building rows\n");
	logDebugging(_logger, "Building rows...");
	for (int i = 0; i < group_count; ++i) {
		char** row = malloc(2 * sizeof(char*));
		if (!row) {
			printf("DEBUG: Failed to allocate memory for row %d\n", i);
			logError(_logger, "Failed to allocate memory for row %d", i);
			// Clean up previously allocated rows
			for (int j = 0; j < i; ++j) {
				free((void*)rows[j][0]);
				free((void*)rows[j][1]);
				free((void*)rows[j]);
			}
			free(rows);
			html_close_dashboard();
			return;
		}
		row[0] = strdup(groups[i].ip);
		char* count_str = malloc(16);
		if (!row[0] || !count_str) {
			printf("DEBUG: Failed to allocate memory for row data %d\n", i);
			logError(_logger, "Failed to allocate memory for row data %d", i);
			free(row);
			// Clean up previously allocated rows
			for (int j = 0; j < i; ++j) {
				free((void*)rows[j][0]);
				free((void*)rows[j][1]);
				free((void*)rows[j]);
			}
			free(rows);
			html_close_dashboard();
			return;
		}
		snprintf(count_str, 16, "%d", groups[i].count);
		row[1] = count_str;
		rows[i] = (const char**)row;
	}
	
	printf("DEBUG: Adding table to HTML\n");
	logDebugging(_logger, "Adding table to HTML...");
	html_add_simple_table(headers, 2, rows, group_count);
	printf("DEBUG: Added group-by table to HTML\n");
	logDebugging(_logger, "Added group-by table to HTML");
	
	// Clean up memory
	printf("DEBUG: Cleaning up memory\n");
	logDebugging(_logger, "Cleaning up memory...");
	for (int i = 0; i < group_count; ++i) {
		free((void*)rows[i][0]);
		free((void*)rows[i][1]);
		free((void*)rows[i]);
	}
	free(rows);

	// Close HTML dashboard
	printf("DEBUG: Closing HTML dashboard\n");
	logDebugging(_logger, "Closing HTML dashboard...");
	html_close_dashboard();
	printf("DEBUG: HTML generation completed\n");
	logDebugging(_logger, "HTML generation completed. Output saved to output.html");
}
