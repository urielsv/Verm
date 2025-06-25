#include "HtmlOutput.h"
#include "../../shared/Logger.h"
#include <string.h>
#include <stdlib.h>

static FILE* html_file = NULL;
static int metrics_count = 0;
static int tables_count = 0;
static int charts_count = 0;

int html_init_dashboard(const char* filename, const char* title) {
    if (!filename || !title) {
        logError(NULL, "Invalid parameters for HTML dashboard initialization");
        return -1;
    }
    
    html_file = fopen(filename, "w");
    if (!html_file) {
        logError(NULL, "Failed to open HTML file: %s", filename);
        return -1;
    }
    
    logInformation(NULL, "Initializing HTML dashboard: %s", filename);
    
    // Write HTML header
    fprintf(html_file, "<!DOCTYPE html>\n");
    fprintf(html_file, "<html lang=\"en\">\n");
    fprintf(html_file, "<head>\n");
    fprintf(html_file, "    <meta charset=\"UTF-8\">\n");
    fprintf(html_file, "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
    fprintf(html_file, "    <title>%s</title>\n", title);
    fprintf(html_file, "    <link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n");
    fprintf(html_file, "    <script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>\n");
    fprintf(html_file, "    <style>\n");
    fprintf(html_file, "        .metric-card { transition: transform 0.2s; }\n");
    fprintf(html_file, "        .metric-card:hover { transform: translateY(-2px); }\n");
    fprintf(html_file, "        .alert-section { margin: 20px 0; }\n");
    fprintf(html_file, "        .chart-container { position: relative; height: 400px; margin: 20px 0; }\n");
    fprintf(html_file, "    </style>\n");
    fprintf(html_file, "</head>\n");
    fprintf(html_file, "<body class=\"bg-light\">\n");
    fprintf(html_file, "    <div class=\"container-fluid\">\n");
    fprintf(html_file, "        <div class=\"row\">\n");
    fprintf(html_file, "            <div class=\"col-12\">\n");
    fprintf(html_file, "                <h1 class=\"text-center my-4\">%s</h1>\n", title);
    fprintf(html_file, "                <div class=\"row\" id=\"metrics-container\">\n");
    
    metrics_count = 0;
    tables_count = 0;
    charts_count = 0;
    
    return 0;
}

int html_add_metric(const char* name, const char* value, const char* description, const char* type) {
    if (!html_file || !name || !value || !description || !type) {
        return -1;
    }
    
    const char* bg_class = "bg-primary";
    const char* text_class = "text-white";
    
    if (strcmp(type, "success") == 0) {
        bg_class = "bg-success";
    } else if (strcmp(type, "warning") == 0) {
        bg_class = "bg-warning";
        text_class = "text-dark";
    } else if (strcmp(type, "danger") == 0) {
        bg_class = "bg-danger";
    } else if (strcmp(type, "info") == 0) {
        bg_class = "bg-info";
    }
    
    fprintf(html_file, "                    <div class=\"col-md-3 col-sm-6 mb-3\">\n");
    fprintf(html_file, "                        <div class=\"card metric-card %s %s\" id=\"metric-%d\">\n", bg_class, text_class, metrics_count);
    fprintf(html_file, "                            <div class=\"card-body text-center\">\n");
    fprintf(html_file, "                                <h5 class=\"card-title\">%s</h5>\n", name);
    fprintf(html_file, "                                <h2 class=\"card-text\">%s</h2>\n", value);
    fprintf(html_file, "                                <p class=\"card-text small\">%s</p>\n", description);
    fprintf(html_file, "                            </div>\n");
    fprintf(html_file, "                        </div>\n");
    fprintf(html_file, "                    </div>\n");
    
    metrics_count++;
    return 0;
}

int html_update_metric(const char* name, const char* value) {
    if (!html_file || !name || !value) {
        return -1;
    }
    
    // This would require JavaScript to update the DOM
    // For now, we'll just log the update
    logDebugging(NULL, "Updating metric %s to %s", name, value);
    return 0;
}

int html_add_table(const char* title, const char** headers, int header_count) {
    if (!html_file || !title || !headers || header_count <= 0) {
        return -1;
    }
    
    fprintf(html_file, "                </div>\n"); // Close metrics row
    fprintf(html_file, "                <div class=\"row mt-4\">\n");
    fprintf(html_file, "                    <div class=\"col-12\">\n");
    fprintf(html_file, "                        <div class=\"card\">\n");
    fprintf(html_file, "                            <div class=\"card-header\">\n");
    fprintf(html_file, "                                <h5 class=\"mb-0\">%s</h5>\n", title);
    fprintf(html_file, "                            </div>\n");
    fprintf(html_file, "                            <div class=\"card-body\">\n");
    fprintf(html_file, "                                <div class=\"table-responsive\">\n");
    fprintf(html_file, "                                    <table class=\"table table-striped table-hover\" id=\"table-%d\">\n", tables_count);
    fprintf(html_file, "                                        <thead class=\"table-dark\">\n");
    fprintf(html_file, "                                            <tr>\n");
    
    for (int i = 0; i < header_count; i++) {
        fprintf(html_file, "                                                <th>%s</th>\n", headers[i]);
    }
    
    fprintf(html_file, "                                            </tr>\n");
    fprintf(html_file, "                                        </thead>\n");
    fprintf(html_file, "                                        <tbody>\n");
    
    tables_count++;
    return 0;
}

int html_add_table_row(const char** values, int value_count) {
    if (!html_file || !values || value_count <= 0) {
        return -1;
    }
    
    fprintf(html_file, "                                            <tr>\n");
    
    for (int i = 0; i < value_count; i++) {
        fprintf(html_file, "                                                <td>%s</td>\n", values[i] ? values[i] : "");
    }
    
    fprintf(html_file, "                                            </tr>\n");
    return 0;
}

int html_add_chart(const char* title, const char* chart_type, const char* data) {
    if (!html_file || !title || !chart_type || !data) {
        return -1;
    }
    
    fprintf(html_file, "                </div>\n"); // Close previous row
    fprintf(html_file, "                <div class=\"row mt-4\">\n");
    fprintf(html_file, "                    <div class=\"col-12\">\n");
    fprintf(html_file, "                        <div class=\"card\">\n");
    fprintf(html_file, "                            <div class=\"card-header\">\n");
    fprintf(html_file, "                                <h5 class=\"mb-0\">%s</h5>\n", title);
    fprintf(html_file, "                            </div>\n");
    fprintf(html_file, "                            <div class=\"card-body\">\n");
    fprintf(html_file, "                                <div class=\"chart-container\">\n");
    fprintf(html_file, "                                    <canvas id=\"chart-%d\"></canvas>\n", charts_count);
    fprintf(html_file, "                                </div>\n");
    fprintf(html_file, "                            </div>\n");
    fprintf(html_file, "                        </div>\n");
    fprintf(html_file, "                    </div>\n");
    fprintf(html_file, "                </div>\n");
    
    // Add JavaScript for chart initialization
    fprintf(html_file, "                <script>\n");
    fprintf(html_file, "                    const ctx%d = document.getElementById('chart-%d').getContext('2d');\n", charts_count, charts_count);
    fprintf(html_file, "                    new Chart(ctx%d, {\n", charts_count);
    fprintf(html_file, "                        type: '%s',\n", chart_type);
    fprintf(html_file, "                        data: %s,\n", data);
    fprintf(html_file, "                        options: {\n");
    fprintf(html_file, "                            responsive: true,\n");
    fprintf(html_file, "                            maintainAspectRatio: false\n");
    fprintf(html_file, "                        }\n");
    fprintf(html_file, "                    });\n");
    fprintf(html_file, "                </script>\n");
    
    charts_count++;
    return 0;
}

int html_add_section(const char* title, const char* content) {
    if (!html_file || !title || !content) {
        return -1;
    }
    
    fprintf(html_file, "                </div>\n"); // Close previous row
    fprintf(html_file, "                <div class=\"row mt-4\">\n");
    fprintf(html_file, "                    <div class=\"col-12\">\n");
    fprintf(html_file, "                        <div class=\"card\">\n");
    fprintf(html_file, "                            <div class=\"card-header\">\n");
    fprintf(html_file, "                                <h5 class=\"mb-0\">%s</h5>\n", title);
    fprintf(html_file, "                            </div>\n");
    fprintf(html_file, "                            <div class=\"card-body\">\n");
    fprintf(html_file, "                                %s\n", content);
    fprintf(html_file, "                            </div>\n");
    fprintf(html_file, "                        </div>\n");
    fprintf(html_file, "                    </div>\n");
    fprintf(html_file, "                </div>\n");
    
    return 0;
}

int html_add_filter_summary(const char* filter_string, int packet_count) {
    if (!html_file || !filter_string) {
        return -1;
    }
    
    char content[1024];
    snprintf(content, sizeof(content), 
             "<p><strong>BPF Filter:</strong> <code>%s</code></p>"
             "<p><strong>Matching Packets:</strong> %d</p>",
             filter_string, packet_count);
    
    return html_add_section("Filter Summary", content);
}

int html_add_protocol_stats(const char* protocol, int count, double percentage) {
    if (!html_file || !protocol) {
        return -1;
    }
    
    char content[512];
    snprintf(content, sizeof(content), 
             "<div class=\"d-flex justify-content-between align-items-center\">"
             "<span>%s</span>"
             "<span>%d packets (%.1f%%)</span>"
             "</div>",
             protocol, count, percentage);
    
    return html_add_section("Protocol Statistics", content);
}

int html_add_top_ips(const char** ips, int* counts, int count) {
    if (!html_file || !ips || !counts || count <= 0) {
        return -1;
    }
    
    const char* headers[] = {"IP Address", "Packet Count"};
    html_add_table("Top IP Addresses", headers, 2);
    
    for (int i = 0; i < count; i++) {
        char count_str[32];
        snprintf(count_str, sizeof(count_str), "%d", counts[i]);
        const char* values[] = {ips[i], count_str};
        html_add_table_row(values, 2);
    }
    
    return 0;
}

int html_add_top_ports(const int* ports, int* counts, int count) {
    if (!html_file || !ports || !counts || count <= 0) {
        return -1;
    }
    
    const char* headers[] = {"Port", "Packet Count"};
    html_add_table("Top Ports", headers, 2);
    
    for (int i = 0; i < count; i++) {
        char port_str[16], count_str[32];
        snprintf(port_str, sizeof(port_str), "%d", ports[i]);
        snprintf(count_str, sizeof(count_str), "%d", counts[i]);
        const char* values[] = {port_str, count_str};
        html_add_table_row(values, 2);
    }
    
    return 0;
}

int html_add_http_stats(const int* status_codes, int* counts, int count) {
    if (!html_file || !status_codes || !counts || count <= 0) {
        return -1;
    }
    
    const char* headers[] = {"Status Code", "Count"};
    html_add_table("HTTP Status Codes", headers, 2);
    
    for (int i = 0; i < count; i++) {
        char status_str[16], count_str[32];
        snprintf(status_str, sizeof(status_str), "%d", status_codes[i]);
        snprintf(count_str, sizeof(count_str), "%d", counts[i]);
        const char* values[] = {status_str, count_str};
        html_add_table_row(values, 2);
    }
    
    return 0;
}

int html_add_alert(const char* title, const char* message, const char* severity) {
    if (!html_file || !title || !message || !severity) {
        return -1;
    }
    
    const char* alert_class = "alert-info";
    
    if (strcmp(severity, "warning") == 0) {
        alert_class = "alert-warning";
    } else if (strcmp(severity, "error") == 0) {
        alert_class = "alert-danger";
    } else if (strcmp(severity, "critical") == 0) {
        alert_class = "alert-danger";
    } else if (strcmp(severity, "success") == 0) {
        alert_class = "alert-success";
    }
    
    fprintf(html_file, "                <div class=\"alert %s alert-dismissible fade show\" role=\"alert\">\n", alert_class);
    fprintf(html_file, "                    <strong>%s</strong> %s\n", title, message);
    fprintf(html_file, "                    <button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"alert\"></button>\n");
    fprintf(html_file, "                </div>\n");
    
    return 0;
}

int html_add_query_section(const char* dsl_query) {
    if (!html_file || !dsl_query) return -1;
    fprintf(html_file, "<h2>Query</h2><pre>%s</pre>\n", dsl_query);
    return 0;
}

int html_add_simple_table(const char** headers, int header_count, const char*** rows, int row_count) {
    if (!html_file || !headers || header_count <= 0) return -1;
    fprintf(html_file, "<h2>Results</h2><table><tr>");
    for (int i = 0; i < header_count; ++i)
        fprintf(html_file, "<th>%s</th>", headers[i]);
    fprintf(html_file, "</tr>\n");
    for (int r = 0; r < row_count; ++r) {
        fprintf(html_file, "<tr>");
        for (int c = 0; c < header_count; ++c)
            fprintf(html_file, "<td>%s</td>", rows[r][c]);
        fprintf(html_file, "</tr>\n");
    }
    fprintf(html_file, "</table>\n");
    return 0;
}

int html_close_dashboard() {
    if (!html_file) {
        return -1;
    }
    
    // Close any open tables
    if (tables_count > 0) {
        fprintf(html_file, "                                        </tbody>\n");
        fprintf(html_file, "                                    </table>\n");
        fprintf(html_file, "                                </div>\n");
        fprintf(html_file, "                            </div>\n");
        fprintf(html_file, "                        </div>\n");
        fprintf(html_file, "                    </div>\n");
        fprintf(html_file, "                </div>\n");
    }
    
    // Close container and add Bootstrap JS
    fprintf(html_file, "            </div>\n");
    fprintf(html_file, "        </div>\n");
    fprintf(html_file, "    </div>\n");
    fprintf(html_file, "    <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.bundle.min.js\"></script>\n");
    fprintf(html_file, "</body>\n");
    fprintf(html_file, "</html>\n");
    
    fclose(html_file);
    html_file = NULL;
    
    logInformation(NULL, "HTML dashboard closed successfully");
    return 0;
} 