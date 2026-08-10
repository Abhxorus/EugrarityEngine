/**
 * @file PlatformUtils.h
 * @brief Funciones de utilidad que interactúan con el sistema operativo (Windows).
 */
#pragma once
#include <string>

class PlatformUtils {
public:
    /**
     * @brief Abre la ventana nativa de Windows para buscar y seleccionar un archivo.
     * @param filter Filtro de extensiones con formato "Nombre\0*.ext\0"
     * @return La ruta absoluta del archivo seleccionado, o un string vacío si se cancela.
     */
    static std::string OpenFileDialog(const char* filter);
};