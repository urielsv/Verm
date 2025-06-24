[![✗](https://img.shields.io/badge/Release-v1.1.0-ffb600.svg?style=for-the-badge)](https://github.com/agustin-golmar/Flex-Bison-Compiler/releases)

[![✗](https://github.com/agustin-golmar/Flex-Bison-Compiler/actions/workflows/pipeline.yaml/badge.svg?branch=production)](https://github.com/agustin-golmar/Flex-Bison-Compiler/actions/workflows/pipeline.yaml)

# Verm - Network Analysis Compiler

Verm es un lenguaje de dominio específico (DSL) diseñado para analizar y filtrar tráfico de red desde archivos de captura PCAP. Permite definir reglas para extraer información específica de diversos protocolos (TCP/IP, HTTP, etc.), generar estadísticas sobre el tráfico, y detectar patrones o anomalías en los paquetes.

## Características Principales

### 🎯 Análisis de Protocolos
- **IPv4**: Extracción de direcciones IP, protocolos, TTL, longitud total
- **TCP**: Puertos, números de secuencia, ventana, flags
- **UDP**: Puertos, longitud de datagrama
- **HTTP**: Métodos, URLs, códigos de estado, headers, cookies

### 🔍 Filtrado y Extracción
- Filtros BPF (Berkeley Packet Filter) para limitar paquetes procesados
- Extracción de campos específicos de protocolos soportados
- Variables para almacenar información extraída
- Tipos de datos: packet, address, integer, boolean, string, timestamp, array

### 📊 Agregación y Estadísticas
- Funciones de agregación: count, sum, avg, min, max
- Agrupación por campos específicos
- Análisis estadístico de tráfico
- Detección de patrones y anomalías

### 🚨 Detección de Anomalías
- Escaneo de puertos
- Ataques DDoS
- Ataques HTTP lentos
- Exfiltración de datos
- Patrones de comunicación periódicos
- Comportamiento anómalo de hosts

### 📈 Reportes y Visualización
- Exportación a JSON para procesamiento posterior
- Dashboard HTML con métricas y gráficos
- Alertas y notificaciones
- Estadísticas detalladas por protocolo

## Instalación

### Prerrequisitos

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y libpcap-dev libjson-c-dev build-essential flex bison
```

#### macOS
```bash
brew install libpcap json-c flex bison
```

### Compilación

```bash
# Clonar el repositorio
git clone <repository-url>
cd Verm

# Instalar dependencias
make install-deps  # Ubuntu/Debian
# o
make install-deps-macos  # macOS

# Compilar el proyecto
make

# Ejecutar con información de debug
make debug

# Compilar versión optimizada
make release
```

## Uso

### Sintaxis del Lenguaje

```vermlang
// Capturar desde archivo PCAP
capture from "traffic.pcap";

// Filtrar paquetes HTTP con errores
filter http.status_code > 400;

// Extraer campos específicos
extract {
    ip.src,
    ip.dst,
    tcp.src_port,
    tcp.dst_port,
    http.method,
    http.url,
    http.status_code
};

// Definir variables
let error_count = count(http.status_code > 400);
let avg_response_time = avg(http.response_time);

// Agrupar por IP origen
group by ip.src {
    let packet_count = count();
    let total_bytes = sum(packet.length);
};

// Alertas
alert when error_count > 10 {
    message: "High HTTP error rate detected";
    severity: "warning";
};

// Exportar resultados
export to "analysis.json";
```

### Ejemplos de Casos de Uso

#### 1. Detectar Paquetes HTTP con Errores
```vermlang
capture from "web_traffic.pcap";
filter http.status_code > 400;
extract { ip.src, http.status_code, http.url };
alert when count() > 5 {
    message: "Multiple HTTP errors detected";
};
```

#### 2. Análisis de Tráfico por IP
```vermlang
capture from "network.pcap";
group by ip.src {
    let packet_count = count();
    let total_bytes = sum(packet.length);
    let avg_packet_size = avg(packet.length);
};
```

#### 3. Detectar Escaneo de Puertos
```vermlang
capture from "scan.pcap";
filter tcp.flags == "SYN";
group by ip.src {
    let unique_ports = count(distinct tcp.dst_port);
    alert when unique_ports > 20 {
        message: "Port scanning detected";
        severity: "critical";
    };
};
```

#### 4. Análisis de Latencia HTTP
```vermlang
capture from "http_traffic.pcap";
filter http.method == "GET";
let avg_latency = avg(http.response_time - http.request_time);
alert when avg_latency > 5000 {
    message: "High HTTP latency detected";
};
```

## Arquitectura del Proyecto

```
src/main/c/
├── frontend/                 # Análisis léxico y sintáctico
│   ├── lexical-analysis/    # Scanner Flex
│   └── syntactic-analysis/  # Parser Bison
├── backend/                 # Backend del compilador
│   ├── semantic-analysis/   # Análisis semántico y tipos
│   ├── code-generation/     # Generación de código C
│   ├── runtime/            # Runtime para procesamiento PCAP
│   ├── domain-specific/    # Análisis de patrones y anomalías
│   └── shared/             # Utilidades compartidas
└── test/c/                 # Test cases
```

### Componentes Principales

#### Frontend
- **LexicalAnalyzer**: Tokenización del código fuente
- **SyntacticAnalyzer**: Análisis sintáctico y construcción del AST
- **AbstractSyntaxTree**: Representación del programa

#### Backend
- **SemanticAnalyzer**: Verificación de tipos y análisis semántico
- **CodeGenerator**: Generación de código C ejecutable
- **PcapRuntime**: Procesamiento de archivos PCAP
- **ProtocolExtractors**: Extracción de campos de protocolos
- **HtmlOutput**: Generación de reportes HTML
- **Calculator**: Análisis de patrones y detección de anomalías

## Casos de Prueba

### Casos de Aceptación
1. ✅ Filtrar paquetes HTTP con código de respuesta > 400
2. ✅ Calcular estadísticas de tráfico por IP origen
3. ✅ Detectar intentos de escaneo de puertos
4. ✅ Identificar las 10 conexiones más lentas
5. ✅ Generar resumen de protocolos utilizados
6. ✅ Detectar transferencias de archivos grandes
7. ✅ Identificar hosts que generan tráfico anómalo
8. ✅ Analizar latencia entre solicitudes y respuestas HTTP
9. ✅ Extraer y analizar cookies HTTP específicas
10. ✅ Detectar patrones de comunicación periódicos

### Casos de Rechazo
1. ✅ Programa malformado sintácticamente
2. ✅ Acceso a campos de protocolos inexistentes
3. ✅ Captura desde interfaz inexistente
4. ✅ Mezcla de tipos de datos incompatibles
5. ✅ Operaciones no permitidas sobre ciertos tipos

## Desarrollo

### Estructura de Desarrollo

```bash
# Compilar con información de debug
make debug

# Ejecutar pruebas
make test

# Verificar memoria
make memcheck

# Análisis estático
make analyze

# Formatear código
make format
```

### Agregar Nuevas Funcionalidades

1. **Nuevos Protocolos**: Extender `ProtocolExtractors.c`
2. **Nuevos Patrones**: Agregar funciones en `Calculator.c`
3. **Nuevos Tipos**: Modificar `TypeSystem.c`
4. **Nueva Sintaxis**: Actualizar `BisonGrammar.y` y `FlexPatterns.l`

### Debugging

```bash
# Compilar con símbolos de debug
make debug

# Ejecutar con valgrind
make memcheck

# Logs detallados
export VERM_LOG_LEVEL=DEBUG
./bin/verm input.vermlang
```

### Test Cases
```bash
# Run acceptance tests
for file in src/test/c/accept/*.verm; do
    ./Compiler "$file"
done

# Run rejection tests (should fail)
for file in src/test/c/reject/*.verm; do
    ./Compiler "$file" 2>/dev/null || echo "Expected failure: $file"
done
```

## Project Structure

```
src/
├── main/c/
│   ├── frontend/           # Lexical and syntactic analysis
│   │   ├── lexical-analysis/
│   │   └── syntactic-analysis/
│   ├── backend/            # Semantic analysis and code generation
│   │   ├── semantic-analysis/
│   │   ├── code-generation/
│   │   └── runtime/
│   └── shared/             # Common utilities
├── test/c/                 # Test cases
│   ├── accept/             # Valid programs
│   └── reject/             # Invalid programs
└── script/                 # Build scripts
```

## Environment Variables

| Name | Default | Description |
|------|---------|-------------|
| `LOG_IGNORED_LEXEMES` | `true` | Log ignored lexemes at DEBUG level |
| `LOGGING_LEVEL` | `INFORMATION` | Minimum logging level (ALL, DEBUGGING, INFORMATION, WARNING, ERROR, CRITICAL) |

## Output Format

The compiler generates HTML dashboards with:
- **Metrics Cards**: Key statistics with color-coded indicators
- **Data Tables**: Detailed packet information
