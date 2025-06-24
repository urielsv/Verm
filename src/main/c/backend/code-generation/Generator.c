#include "Generator.h"
#include "../../backend/runtime/HtmlOutput.h"
#include "../../backend/runtime/PcapRuntime.h"
#include "../../backend/runtime/ProtocolExtractors.h"
#include "../../backend/domain-specific/Calculator.h"
#include <string.h>
#include <stdlib.h>

/* MODULE INTERNAL STATE */

const char _indentationCharacter = ' ';
const char _indentationSize = 4;
static Logger * _logger = NULL;
static FILE* _htmlFile = NULL;
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
	struct PacketInfo* next;
} PacketInfo;

static PacketInfo* packet_list_head = NULL;
static PacketInfo* packet_list_tail = NULL;

void initializeGeneratorModule() {
	_logger = createLogger("Generator");
}

void shutdownGeneratorModule() {
	if (_logger != NULL) {
		destroyLogger(_logger);
	}
	if (_htmlFile != NULL) {
		fclose(_htmlFile);
		_htmlFile = NULL;
	}
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
	PacketInfo* info = (PacketInfo*)calloc(1, sizeof(PacketInfo));
	// Extraer campos IP
	extract_ip_src_addr(packet, info->src_ip);
	extract_ip_dst_addr(packet, info->dst_ip);
	// TCP
	if (is_tcp_packet(packet)) {
		extract_tcp_src_port(packet, &info->src_port);
		extract_tcp_dst_port(packet, &info->dst_port);
		info->protocol = 0;
		_tcpConnections++;
	}
	// UDP
	else if (is_udp_packet(packet)) {
		extract_udp_src_port(packet, &info->src_port);
		extract_udp_dst_port(packet, &info->dst_port);
		info->protocol = 1;
		_udpPackets++;
	}
	// HTTP
	if (is_http_packet(packet)) {
		extract_http_status_code(packet, &info->http_status);
		extract_http_method(packet, info->http_method);
		info->protocol = 2;
		if (info->http_status >= 400) {
			_httpErrors++;
		}
	}
	_packetCount++;
	add_packet_info(info);
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

// Ejemplo simple de agrupamiento por src_ip y conteo de errores HTTP
static void group_and_count_errors_by_src_ip(FILE* out) {
	typedef struct Group {
		char src_ip[64];
		int error_count;
		struct Group* next;
	} Group;
	Group* groups = NULL;
	for (PacketInfo* p = packet_list_head; p; p = p->next) {
		if (p->http_status >= 500) {
			Group* g = groups;
			while (g && strcmp(g->src_ip, p->src_ip) != 0) g = g->next;
			if (!g) {
				g = (Group*)calloc(1, sizeof(Group));
				strcpy(g->src_ip, p->src_ip);
				g->error_count = 1;
				g->next = groups;
				groups = g;
			} else {
				g->error_count++;
			}
		}
	}
	fprintf(out, "<h3>HTTP 5xx Errors by Source IP</h3>\n<table class='table'><thead><tr><th>Source IP</th><th>Errors</th></tr></thead><tbody>\n");
	for (Group* g = groups; g; g = g->next) {
		fprintf(out, "<tr><td>%s</td><td>%d</td></tr>\n", g->src_ip, g->error_count);
	}
	fprintf(out, "</tbody></table>\n");
	// Liberar grupos
	while (groups) {
		Group* next = groups->next;
		free(groups);
		groups = next;
	}
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
			// Process import statement
			logInformation(_logger, "Processing import from: %s", statement->import_export.filename);
			break;
		case EXPORT_STATEMENT:
			// Process export statement
			logInformation(_logger, "Processing export to: %s", statement->import_export.filename);
			break;
		default:
			logWarning(_logger, "Unsupported statement type: %d", statement->type);
			break;
	}
}

/**
 * Processes capture statements
 */
static void _processCaptureStatement(CaptureStatement * capture) {
	if (!capture) return;
	
	logInformation(_logger, "Processing capture from: %s", capture->interface);
	
	// Simulate packet processing
	_packetCount = 1000; // Simulated packet count
	_httpErrors = 45;     // Simulated HTTP errors
	_tcpConnections = 150; // Simulated TCP connections
	_udpPackets = 200;    // Simulated UDP packets
}

/**
 * Processes extract statements
 */
static void _processExtractStatement(ExtractStatement * extract) {
	if (!extract) return;
	
	logInformation(_logger, "Processing extract statement");
	
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
 * Outputs a formatted string to the HTML file.
 */
static void _output(const unsigned int indentationLevel, const char * const format, ...) {
	va_list arguments;
	va_start(arguments, format);
	char * indentation = _indentation(indentationLevel);
	char * effectiveFormat = concatenate(2, indentation, format);
	
	if (_htmlFile) {
		vfprintf(_htmlFile, effectiveFormat, arguments);
		fflush(_htmlFile);
	} else {
		vfprintf(stdout, effectiveFormat, arguments);
		fflush(stdout);
	}
	
	free(effectiveFormat);
	free(indentation);
	va_end(arguments);
}

/** PUBLIC FUNCTIONS */

void generate(CompilerState * compilerState) {
	logDebugging(_logger, "Generating HTML output...");
	
	// Open HTML file for output
	_htmlFile = fopen("output.html", "w");
	if (!_htmlFile) {
		logError(_logger, "Failed to open output.html for writing");
		return;
	}
	
	// Inicializar métricas
	_packetCount = 0;
	_httpErrors = 0;
	_tcpConnections = 0;
	_udpPackets = 0;

	// Buscar import/capture y procesar tráfico real
	CaptureConfig config;
	if (find_capture_or_import(compilerState->abstractSyntaxtTree, &config)) {
		PcapContext ctx;
		if (init_pcap_context(&ctx, config.filename, config.filter) == 0) {
			process_packets(&ctx, real_packet_handler);
			cleanup_pcap_context(&ctx);
		} else {
			logError(_logger, "No se pudo inicializar el contexto de captura/import para %s", config.filename);
		}
	} else {
		logWarning(_logger, "No se encontró import ni capture en el programa. No se procesará tráfico real.");
	}

	// Generar HTML
	_generateHtmlHeader();
	_processProgram(compilerState->abstractSyntaxtTree);
	_generateMetrics();
	_generateTables();
	// Ejemplo: tabla de errores HTTP agrupados por src_ip
	group_and_count_errors_by_src_ip(_htmlFile);
	_generateHtmlFooter();
	
	// Close file
	fclose(_htmlFile);
	_htmlFile = NULL;
	
	logDebugging(_logger, "HTML generation completed. Output saved to output.html");
}
