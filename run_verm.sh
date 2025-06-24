#!/bin/bash

# Script para ejecutar archivos Verm y generar HTML
# Uso: ./run_verm.sh <archivo.verm>

if [ $# -eq 0 ]; then
    echo "Uso: $0 <archivo.verm>"
    echo "Ejemplo: $0 src/test/c/accept/08-simple-capture"
    exit 1
fi

VERM_FILE="$1"

if [ ! -f "$VERM_FILE" ]; then
    echo "Error: El archivo '$VERM_FILE' no existe"
    exit 1
fi

echo "Compilando y ejecutando: $VERM_FILE"

# Compilar el proyecto
make clean
make

if [ $? -ne 0 ]; then
    echo "Error: Falló la compilación"
    exit 1
fi

# Ejecutar el compilador con el archivo Verm por stdin
./Compiler < "$VERM_FILE"

if [ $? -eq 0 ]; then
    echo "✅ Análisis completado exitosamente"
    if [ -f "output.html" ]; then
        echo "📄 Reporte HTML generado: output.html"
        echo "🌐 Abriendo en navegador..."
        open output.html 2>/dev/null || xdg-open output.html 2>/dev/null || echo "Abre output.html manualmente en tu navegador"
    else
        echo "⚠️  No se generó output.html"
    fi
else
    echo "❌ Error durante la ejecución"
    exit 1
fi 