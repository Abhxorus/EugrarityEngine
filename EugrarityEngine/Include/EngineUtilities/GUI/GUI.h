#pragma once
#include "Prerequisites.h"

#include "imgui.h"
#include <imgui_internal.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "ImGuizmo.h"

class Viewport;
class Window;
class Device;
class DeviceContext;
class Actor;

class
    GUI {
public:
    GUI() = default;
    ~GUI() = default;

    void
        awake();

    void
        init(Window& window, Device& device, DeviceContext& deviceContext);

    void
        update(Viewport& viewport, Window& window);

    void
        render();

    void
        destroy();

    void
        ToolBar();


    void
        closeApp();

    void
        toolTipData();

    void
        appleLiquidStyle(float opacity /*0..1f*/, ImVec4 accent /*=#0A84FF*/);

    void
        vec3Control(const std::string& label,
            float* values,
            float resetValues = 0.0f,
            float columnWidth = 100.0f);

    void
        inspectorGeneral(EU::TSharedPointer<Actor> actor);

    void
        inspectorContainer(EU::TSharedPointer<Actor> actor);

    void
        outliner(const std::vector<EU::TSharedPointer<Actor>>& actors);

    void
        editTransform(const XMMATRIX& view, const XMMATRIX& projection, EU::TSharedPointer<Actor> actor);

    void
        drawGizmoToolbar();

    // Crea una función auxiliar para convertir XMMATRIX a lo que ImGuizmo quiere
    void ToFloatArray(const XMMATRIX& mat, float* dest) {
        XMFLOAT4X4 temp;
        XMStoreFloat4x4(&temp, mat);
        memcpy(dest, &temp, sizeof(float) * 16);
    }

private:
    bool checkboxValue = true;
    bool checkboxValue2 = false;
    std::vector<const char*> m_objectsNames;
    std::vector<const char*> m_tooltips;

    bool show_exit_popup = false; // Variable de estado para el popup


public:
    int selectedActorIndex = -1;
};