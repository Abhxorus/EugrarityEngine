/**
 * @file PlatformUtils.cpp
 * @brief Implementación de las utilidades de plataforma.
 */
#include "EngineUtilities/Utilities/PlatformUtils.h"
#include <windows.h>
#include <commdlg.h> // Necesario para OPENFILENAME

std::string PlatformUtils::OpenFileDialog(const char* filter) {
    OPENFILENAMEA ofn;       // Estructura de Windows para el cuadro de diálogo
    CHAR szFile[260] = { 0 }; // Buffer para guardar la ruta elegida

    // Limpiamos la memoria de la estructura para evitar basura
    ZeroMemory(&ofn, sizeof(OPENFILENAMEA));

    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = NULL; // Sin ventana padre forzada
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    // OFN_NOCHANGEDIR es vital: evita que Windows cambie la ruta base ("Working Directory") de tu motor al buscar archivos.
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    // Si el usuario selecciona un archivo y le da a "Abrir"
    if (GetOpenFileNameA(&ofn) == TRUE) {
        return std::string(ofn.lpstrFile);
    }

    // Si el usuario cierra la ventana o cancela
    return "";
}