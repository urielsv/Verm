#include "HtmlOutput.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

static FILE* html_file = NULL;
static int table_active = 0;

int html_init_dashboard(const char* filename, const char* title) {
    html_file = fopen(filename, "w");
    if (!html_file) {
        return -1;
    }
    
    time_t now = time(NULL);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    fprintf(html_file, "<!DOCTYPE html>\n");
    fprintf(html_file, "<html lang=\"en\">\n");
    fprintf(html_file, "<head>\n");
    fprintf(html_file, "    <meta charset=\"UTF-8\">\n");
    fprintf(html_file, "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
    fprintf(html_file, "    <title>%s</title>\n", title);
    fprintf(html_file, "    <style>\n");
    fprintf(html_file, "        * { margin: 0; padding: 0; box-sizing: border-box; }\n");
    fprintf(html_file, "        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #f5f5f5; }\n");
    fprintf(html_file, "        .header { background: linear-gradient(135deg, #667eea 0%%, #764ba2 100%%); color: white; padding: 2rem; text-align: center; }\n");
    fprintf(html_file, "        .header h1 { font-size: 2.5rem; margin-bottom: 0.5rem; }\n");
    fprintf(html_file, "        .header p { opacity: 0.9; }\n");
    fprintf(html_file, "        .container { max-width: 1200px; margin: 0 auto; padding: 2rem; }\n");
    fprintf(html_file, "        .metrics-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 1.5rem; margin-bottom: 2rem; }\n");
    fprintf(html_file, "        .metric-card { background: white; padding: 1.5rem; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }\n");
    fprintf(html_file, "        .metric-card.success { border-left: 4px solid #28a745; }\n");
    fprintf(html_file, "        .metric-card.warning { border-left: 4px solid #ffc107; }\n");
    fprintf(html_file, "        .metric-card.danger { border-left: 4px solid #dc3545; }\n");
    fprintf(html_file, "        .metric-card.info { border-left: 4px solid #17a2b8; }\n");
    fprintf(html_file, "        .metric-value { font-size: 2rem; font-weight: bold; margin-bottom: 0.5rem; }\n");
    fprintf(html_file, "        .metric-title { font-size: 1.1rem; font-weight: 600; margin-bottom: 0.5rem; }\n");
    fprintf(html_file, "        .metric-description { color: #666; font-size: 0.9rem; }\n");
    fprintf(html_file, "        .table-section { background: white; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); margin-bottom: 2rem; }\n");
    fprintf(html_file, "        .table-header { padding: 1.5rem; border-bottom: 1px solid #eee; }\n");
    fprintf(html_file, "        .table-header h2 { color: #333; }\n");
    fprintf(html_file, "        table { width: 100%%; border-collapse: collapse; }\n");
    fprintf(html_file, "        th, td { padding: 1rem; text-align: left; border-bottom: 1px solid #eee; }\n");
    fprintf(html_file, "        th { background: #f8f9fa; font-weight: 600; color: #495057; }\n");
    fprintf(html_file, "        tr:hover { background: #f8f9fa; }\n");
    fprintf(html_file, "        .footer { text-align: center; padding: 2rem; color: #666; }\n");
    fprintf(html_file, "    </style>\n");
    fprintf(html_file, "</head>\n");
    fprintf(html_file, "<body>\n");
    fprintf(html_file, "    <div class=\"header\">\n");
    fprintf(html_file, "        <h1>%s</h1>\n", title);
    fprintf(html_file, "        <p>Generated on %s</p>\n", time_str);
    fprintf(html_file, "    </div>\n");
    fprintf(html_file, "    <div class=\"container\">\n");
    fprintf(html_file, "        <div class=\"metrics-grid\">\n");
    
    return 0;
}

void html_add_metric(const char* title, const char* value, const char* description, const char* color) {
    if (!html_file) return;
    
    fprintf(html_file, "            <div class=\"metric-card %s\">\n", color);
    fprintf(html_file, "                <div class=\"metric-value\">%s</div>\n", value);
    fprintf(html_file, "                <div class=\"metric-title\">%s</div>\n", title);
    fprintf(html_file, "                <div class=\"metric-description\">%s</div>\n", description);
    fprintf(html_file, "            </div>\n");
}

void html_start_table(const char* title, const char** headers, int num_headers) {
    if (!html_file) return;
    
    fprintf(html_file, "        </div>\n"); // Close metrics-grid
    fprintf(html_file, "        <div class=\"table-section\">\n");
    fprintf(html_file, "            <div class=\"table-header\">\n");
    fprintf(html_file, "                <h2>%s</h2>\n", title);
    fprintf(html_file, "            </div>\n");
    fprintf(html_file, "            <table>\n");
    fprintf(html_file, "                <thead>\n");
    fprintf(html_file, "                    <tr>\n");
    
    for (int i = 0; i < num_headers; i++) {
        fprintf(html_file, "                        <th>%s</th>\n", headers[i]);
    }
    
    fprintf(html_file, "                    </tr>\n");
    fprintf(html_file, "                </thead>\n");
    fprintf(html_file, "                <tbody>\n");
    
    table_active = 1;
}

void html_add_table_row(const char** values, int num_values) {
    if (!html_file || !table_active) return;
    
    fprintf(html_file, "                    <tr>\n");
    for (int i = 0; i < num_values; i++) {
        fprintf(html_file, "                        <td>%s</td>\n", values[i] ? values[i] : "");
    }
    fprintf(html_file, "                    </tr>\n");
}

void html_end_table(void) {
    if (!html_file) return;
    
    fprintf(html_file, "                </tbody>\n");
    fprintf(html_file, "            </table>\n");
    fprintf(html_file, "        </div>\n");
    
    table_active = 0;
}

void html_close_dashboard(void) {
    if (!html_file) return;
    
    if (table_active) {
        html_end_table();
    }
    
    fprintf(html_file, "    </div>\n");
    fprintf(html_file, "    <div class=\"footer\">\n");
    fprintf(html_file, "        <p>Generated by Verm Network Analysis Tool</p>\n");
    fprintf(html_file, "    </div>\n");
    fprintf(html_file, "</body>\n");
    fprintf(html_file, "</html>\n");
    
    fclose(html_file);
    html_file = NULL;
} 