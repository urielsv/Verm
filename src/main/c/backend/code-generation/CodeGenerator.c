#include "CodeGenerator.h"
#include "../../shared/Logger.h"
#include <string.h>

int generate_pcap_processor(Program* root, FILE* output) {
    if (!root || !output) {
        logError(NULL, "Invalid parameters for code generation");
        return -1;
    }
    
    logInformation(NULL, "Starting code generation for PCAP processor");
    
    // Generate includes
    fprintf(output, "#include <pcap.h>\n");
    fprintf(output, "#include <stdio.h>\n");
    fprintf(output, "#include <stdlib.h>\n");
    fprintf(output, "#include <string.h>\n");
    fprintf(output, "#include <time.h>\n");
    fprintf(output, "#include <json-c/json.h>\n");
    fprintf(output, "#include \"PcapRuntime.h\"\n");
    fprintf(output, "#include \"HtmlOutput.h\"\n");
    fprintf(output, "#include \"ProtocolExtractors.h\"\n\n");
    
    // Generate global variables for aggregation
    fprintf(output, "// Global variables for aggregation\n");
    fprintf(output, "struct json_object* json_output;\n");
    fprintf(output, "struct json_object* packet_data;\n");
    fprintf(output, "struct json_object* statistics;\n\n");
    
    // Generate main function
    if (generate_main_function(root, output) != 0) {
        return -1;
    }
    
    // Generate helper functions
    generate_helper_functions(output);
    
    logInformation(NULL, "Code generation completed successfully");
    return 0;
}

int generate_filter_code(Condition* filter, FILE* output) {
    if (!filter || !output) {
        return -1;
    }
    
    logDebugging(NULL, "Generating filter code");
    
    switch (filter->type) {
        case COMPARISON:
            fprintf(output, "    // Filter condition: ");
            generate_expression_code(filter->comparison.left, output);
            fprintf(output, " %s ", get_comparison_operator(filter->comparison.op));
            generate_expression_code(filter->comparison.right, output);
            fprintf(output, "\n");
            break;
            
        case LOGICAL_AND:
            fprintf(output, "    // AND condition\n");
            fprintf(output, "    if (");
            generate_condition_code(filter->logical.left, output);
            fprintf(output, " && ");
            generate_condition_code(filter->logical.right, output);
            fprintf(output, ") {\n");
            break;
            
        case LOGICAL_OR:
            fprintf(output, "    // OR condition\n");
            fprintf(output, "    if (");
            generate_condition_code(filter->logical.left, output);
            fprintf(output, " || ");
            generate_condition_code(filter->logical.right, output);
            fprintf(output, ") {\n");
            break;
            
        default:
            fprintf(output, "    // TODO: Implement filter condition\n");
            break;
    }
    
    return 0;
}

int generate_extraction_code(ExtractStatement* extract, FILE* output) {
    if (!extract || !output) {
        return -1;
    }
    
    logDebugging(NULL, "Generating extraction code");
    
    FieldList* current = extract->fields;
    while (current) {
        fprintf(output, "    // Extract %s.%s\n", current->field->protocol, current->field->name);
        generate_field_extraction_code(current->field, output);
        current = current->next;
    }
    
    return 0;
}

int generate_aggregation_code(Expression* agg, FILE* output) {
    if (!agg || !output) {
        return -1;
    }
    
    logDebugging(NULL, "Generating aggregation code");
    
    switch (agg->type) {
        case AGGREGATION_COUNT:
            fprintf(output, "    // Count aggregation\n");
            fprintf(output, "    count++;\n");
            break;
            
        case AGGREGATION_SUM:
            fprintf(output, "    // Sum aggregation\n");
            fprintf(output, "    sum += ");
            generate_expression_code(agg->aggregation, output);
            fprintf(output, ";\n");
            break;
            
        case AGGREGATION_AVG:
            fprintf(output, "    // Average aggregation\n");
            fprintf(output, "    sum += ");
            generate_expression_code(agg->aggregation, output);
            fprintf(output, ";\n");
            fprintf(output, "    count++;\n");
            break;
            
        case AGGREGATION_MIN:
            fprintf(output, "    // Min aggregation\n");
            fprintf(output, "    if (");
            generate_expression_code(agg->aggregation, output);
            fprintf(output, " < min) min = ");
            generate_expression_code(agg->aggregation, output);
            fprintf(output, ";\n");
            break;
            
        case AGGREGATION_MAX:
            fprintf(output, "    // Max aggregation\n");
            fprintf(output, "    if (");
            generate_expression_code(agg->aggregation, output);
            fprintf(output, " > max) max = ");
            generate_expression_code(agg->aggregation, output);
            fprintf(output, ";\n");
            break;
            
        default:
            fprintf(output, "    // TODO: Implement aggregation\n");
            break;
    }
    
    return 0;
}

int generate_main_function(Program* root, FILE* output) {
    if (!root || !output) {
        return -1;
    }
    
    fprintf(output, "int main() {\n");
    fprintf(output, "    // Initialize PCAP context\n");
    fprintf(output, "    PcapContext ctx;\n");
    fprintf(output, "    if (init_pcap_context(&ctx, \"input.pcap\", \"\") != 0) {\n");
    fprintf(output, "        fprintf(stderr, \"Failed to initialize PCAP context\\n\");\n");
    fprintf(output, "        return 1;\n");
    fprintf(output, "    }\n\n");
    
    fprintf(output, "    // Initialize JSON output\n");
    fprintf(output, "    json_output = json_object_new_object();\n");
    fprintf(output, "    packet_data = json_object_new_array();\n");
    fprintf(output, "    statistics = json_object_new_object();\n");
    fprintf(output, "    json_object_object_add(json_output, \"packets\", packet_data);\n");
    fprintf(output, "    json_object_object_add(json_output, \"statistics\", statistics);\n\n");
    
    fprintf(output, "    // Initialize HTML dashboard\n");
    fprintf(output, "    if (html_init_dashboard(\"output.html\", \"Network Analysis Report\") != 0) {\n");
    fprintf(output, "        fprintf(stderr, \"Failed to initialize HTML output\\n\");\n");
    fprintf(output, "        return 1;\n");
    fprintf(output, "    }\n\n");
    
    fprintf(output, "    // Add metrics\n");
    fprintf(output, "    html_add_metric(\"Total Packets\", \"0\", \"Number of processed packets\", \"info\");\n");
    fprintf(output, "    html_add_metric(\"HTTP Errors\", \"0\", \"HTTP error responses\", \"danger\");\n");
    fprintf(output, "    html_add_metric(\"TCP Connections\", \"0\", \"TCP connections detected\", \"success\");\n");
    fprintf(output, "    html_add_metric(\"UDP Packets\", \"0\", \"UDP packets detected\", \"warning\");\n\n");
    
    fprintf(output, "    // Initialize counters\n");
    fprintf(output, "    int total_packets = 0;\n");
    fprintf(output, "    int http_errors = 0;\n");
    fprintf(output, "    int tcp_connections = 0;\n");
    fprintf(output, "    int udp_packets = 0;\n\n");
    
    fprintf(output, "    // Process packets\n");
    fprintf(output, "    process_packets(&ctx, packet_handler);\n\n");
    
    fprintf(output, "    // Update metrics\n");
    fprintf(output, "    char metric_value[32];\n");
    fprintf(output, "    sprintf(metric_value, \"%%d\", total_packets);\n");
    fprintf(output, "    html_update_metric(\"Total Packets\", metric_value);\n");
    fprintf(output, "    sprintf(metric_value, \"%%d\", http_errors);\n");
    fprintf(output, "    html_update_metric(\"HTTP Errors\", metric_value);\n");
    fprintf(output, "    sprintf(metric_value, \"%%d\", tcp_connections);\n");
    fprintf(output, "    html_update_metric(\"TCP Connections\", metric_value);\n");
    fprintf(output, "    sprintf(metric_value, \"%%d\", udp_packets);\n");
    fprintf(output, "    html_update_metric(\"UDP Packets\", metric_value);\n\n");
    
    fprintf(output, "    // Write JSON output\n");
    fprintf(output, "    FILE* json_file = fopen(\"output.json\", \"w\");\n");
    fprintf(output, "    if (json_file) {\n");
    fprintf(output, "        fprintf(json_file, \"%%s\", json_object_to_json_string_ext(json_output, JSON_C_TO_STRING_PRETTY));\n");
    fprintf(output, "        fclose(json_file);\n");
    fprintf(output, "    }\n\n");
    
    fprintf(output, "    // Close outputs\n");
    fprintf(output, "    html_close_dashboard();\n");
    fprintf(output, "    cleanup_pcap_context(&ctx);\n");
    fprintf(output, "    json_object_put(json_output);\n\n");
    
    fprintf(output, "    printf(\"Analysis completed. Check output.html and output.json for results.\\n\");\n");
    fprintf(output, "    return 0;\n");
    fprintf(output, "}\n\n");
    
    return 0;
}

void generate_helper_functions(FILE* output) {
    fprintf(output, "// Packet handler function\n");
    fprintf(output, "void packet_handler(u_char* user, const struct pcap_pkthdr* header, const u_char* packet) {\n");
    fprintf(output, "    static int packet_count = 0;\n");
    fprintf(output, "    packet_count++;\n\n");
    
    fprintf(output, "    // Create packet object\n");
    fprintf(output, "    struct json_object* packet_obj = json_object_new_object();\n");
    fprintf(output, "    json_object_object_add(packet_obj, \"number\", json_object_new_int(packet_count));\n\n");
    
    fprintf(output, "    // Extract basic packet info\n");
    fprintf(output, "    char timestamp[64];\n");
    fprintf(output, "    extract_timestamp(header, timestamp);\n");
    fprintf(output, "    json_object_object_add(packet_obj, \"timestamp\", json_object_new_string(timestamp));\n");
    fprintf(output, "    json_object_object_add(packet_obj, \"length\", json_object_new_int(header->len));\n\n");
    
    fprintf(output, "    // Extract IP info if available\n");
    fprintf(output, "    if (is_ipv4_packet(packet)) {\n");
    fprintf(output, "        char src_ip[INET_ADDRSTRLEN], dst_ip[INET_ADDRSTRLEN];\n");
    fprintf(output, "        if (extract_ip_src_addr(packet, src_ip) == 0) {\n");
    fprintf(output, "            json_object_object_add(packet_obj, \"src_ip\", json_object_new_string(src_ip));\n");
    fprintf(output, "        }\n");
    fprintf(output, "        if (extract_ip_dst_addr(packet, dst_ip) == 0) {\n");
    fprintf(output, "            json_object_object_add(packet_obj, \"dst_ip\", json_object_new_string(dst_ip));\n");
    fprintf(output, "        }\n\n");
    
    fprintf(output, "        // Extract TCP info if available\n");
    fprintf(output, "        if (is_tcp_packet(packet)) {\n");
    fprintf(output, "            int src_port, dst_port;\n");
    fprintf(output, "            if (extract_tcp_src_port(packet, &src_port) == 0) {\n");
    fprintf(output, "                json_object_object_add(packet_obj, \"src_port\", json_object_new_int(src_port));\n");
    fprintf(output, "            }\n");
    fprintf(output, "            if (extract_tcp_dst_port(packet, &dst_port) == 0) {\n");
    fprintf(output, "                json_object_object_add(packet_obj, \"dst_port\", json_object_new_int(dst_port));\n");
    fprintf(output, "            }\n");
    fprintf(output, "            json_object_object_add(packet_obj, \"protocol\", json_object_new_string(\"TCP\"));\n");
    fprintf(output, "        }\n");
    fprintf(output, "        // Extract UDP info if available\n");
    fprintf(output, "        else if (is_udp_packet(packet)) {\n");
    fprintf(output, "            int src_port, dst_port;\n");
    fprintf(output, "            if (extract_udp_src_port(packet, &src_port) == 0) {\n");
    fprintf(output, "                json_object_object_add(packet_obj, \"src_port\", json_object_new_int(src_port));\n");
    fprintf(output, "            }\n");
    fprintf(output, "            if (extract_udp_dst_port(packet, &dst_port) == 0) {\n");
    fprintf(output, "                json_object_object_add(packet_obj, \"dst_port\", json_object_new_int(dst_port));\n");
    fprintf(output, "            }\n");
    fprintf(output, "            json_object_object_add(packet_obj, \"protocol\", json_object_new_string(\"UDP\"));\n");
    fprintf(output, "        }\n");
    fprintf(output, "    }\n\n");
    
    fprintf(output, "    // Extract HTTP info if available\n");
    fprintf(output, "    if (is_http_packet(packet)) {\n");
    fprintf(output, "        int status_code;\n");
    fprintf(output, "        if (extract_http_status_code(packet, &status_code) == 0) {\n");
    fprintf(output, "            json_object_object_add(packet_obj, \"http_status\", json_object_new_int(status_code));\n");
    fprintf(output, "            if (status_code >= 400) {\n");
    fprintf(output, "                json_object_object_add(packet_obj, \"http_error\", json_object_new_boolean(1));\n");
    fprintf(output, "            }\n");
    fprintf(output, "        }\n");
    fprintf(output, "        char method[16];\n");
    fprintf(output, "        if (extract_http_method(packet, method) == 0) {\n");
    fprintf(output, "            json_object_object_add(packet_obj, \"http_method\", json_object_new_string(method));\n");
    fprintf(output, "        }\n");
    fprintf(output, "        json_object_object_add(packet_obj, \"protocol\", json_object_new_string(\"HTTP\"));\n");
    fprintf(output, "    }\n\n");
    
    fprintf(output, "    // Add packet to array\n");
    fprintf(output, "    json_object_array_add(packet_data, packet_obj);\n");
    fprintf(output, "}\n\n");
}

void generate_expression_code(Expression* expr, FILE* output) {
    if (!expr) return;
    
    switch (expr->type) {
        case ADDITION:
            generate_expression_code(expr->leftExpression, output);
            fprintf(output, " + ");
            generate_expression_code(expr->rightExpression, output);
            break;
            
        case SUBTRACTION:
            generate_expression_code(expr->leftExpression, output);
            fprintf(output, " - ");
            generate_expression_code(expr->rightExpression, output);
            break;
            
        case MULTIPLICATION:
            generate_expression_code(expr->leftExpression, output);
            fprintf(output, " * ");
            generate_expression_code(expr->rightExpression, output);
            break;
            
        case DIVISION:
            generate_expression_code(expr->leftExpression, output);
            fprintf(output, " / ");
            generate_expression_code(expr->rightExpression, output);
            break;
            
        case FACTOR:
            generate_factor_code(expr->factor, output);
            break;
            
        default:
            fprintf(output, "0");
            break;
    }
}

void generate_factor_code(Factor* factor, FILE* output) {
    if (!factor) return;
    
    switch (factor->type) {
        case CONSTANT:
            if (factor->constant->value.type == INTEGER_TYPE) {
                fprintf(output, "%d", factor->constant->value.integer);
            } else if (factor->constant->value.type == STRING_TYPE) {
                fprintf(output, "\"%s\"", factor->constant->value.string);
            }
            break;
            
        case FIELD:
            generate_field_code(factor->field, output);
            break;
            
        case VARIABLE_REFERENCE:
            fprintf(output, "%s", factor->variable_name);
            break;
            
        default:
            fprintf(output, "0");
            break;
    }
}

void generate_field_code(Field* field, FILE* output) {
    if (!field) return;
    
    fprintf(output, "extract_%s_%s(packet, &temp_value)", field->protocol, field->name);
}

void generate_condition_code(Condition* condition, FILE* output) {
    if (!condition) return;
    
    switch (condition->type) {
        case COMPARISON:
            generate_expression_code(condition->comparison.left, output);
            fprintf(output, " %s ", get_comparison_operator(condition->comparison.op));
            generate_expression_code(condition->comparison.right, output);
            break;
            
        case LOGICAL_AND:
            generate_condition_code(condition->logical.left, output);
            fprintf(output, " && ");
            generate_condition_code(condition->logical.right, output);
            break;
            
        case LOGICAL_OR:
            generate_condition_code(condition->logical.left, output);
            fprintf(output, " || ");
            generate_condition_code(condition->logical.right, output);
            break;
            
        default:
            fprintf(output, "true");
            break;
    }
}

void generate_field_extraction_code(Field* field, FILE* output) {
    if (!field) return;
    
    fprintf(output, "    int temp_value;\n");
    fprintf(output, "    if (extract_%s_%s(packet, &temp_value) == 0) {\n", field->protocol, field->name);
    fprintf(output, "        json_object_object_add(packet_obj, \"%s_%s\", json_object_new_int(temp_value));\n", field->protocol, field->name);
    fprintf(output, "    }\n");
}

const char* get_comparison_operator(ComparisonOperator op) {
    switch (op) {
        case EQUALS_OP: return "==";
        case NOT_EQUALS_OP: return "!=";
        case LESS_THAN_OP: return "<";
        case GREATER_THAN_OP: return ">";
        case GREATER_THAN_OR_EQUALS_OP: return ">=";
        case LESS_THAN_OR_EQUALS_OP: return "<=";
        default: return "==";
    }
} 