#include "EngineUtilities\GUI\GUI.h"
#include "Viewport.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "MeshComponent.h"
#include "ECS\Actor.h"
//#include "imgui_internal.h"
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
void
GUI::init(Window& window, Device& device, DeviceContext& deviceContext) {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	appleLiquidStyle(0.72f, ImVec4(0.0f, 0.515f, 1.0f, 1.0f));

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(window.m_hWnd);
	ImGui_ImplDX11_Init(device.m_device, deviceContext.m_deviceContext);

	// Init ToolTips
	toolTipData();

	selectedActorIndex = 0;
}

void
GUI::update(Viewport& viewport, Window& window) {
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuizmo::BeginFrame();
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetRect(0, 0, (float)window.m_width, (float)window.m_height);

	// In Program always
	ToolBar();
	closeApp();
	drawGizmoToolbar();
}

void
GUI::render() {
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	ImGuiIO& io = ImGui::GetIO();
	// Update and Render additional Platform Windows
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void
GUI::destroy() {
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void
GUI::vec3Control(const std::string& label, float* values, float resetValue, float columnWidth) {
	ImGuiIO& io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0];

	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

	float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("X", buttonSize)) values[0] = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Y", buttonSize)) values[1] = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Y", &values[1], 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Z", buttonSize)) values[2] = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Z", &values[2], 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();
	ImGui::Columns(1);

	ImGui::PopID();
}

void
GUI::toolTipData() {
}

void
GUI::appleLiquidStyle(float opacity, ImVec4 accent) {
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// Geometría suave tipo macOS
	style.WindowRounding = 14.0f;
	style.ChildRounding = 14.0f;
	style.PopupRounding = 14.0f;
	style.FrameRounding = 10.0f;
	style.GrabRounding = 10.0f;
	style.ScrollbarRounding = 12.0f;
	style.TabRounding = 10.0f;

	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.TabBorderSize = 0.0f;

	style.WindowPadding = ImVec2(14, 12);
	style.FramePadding = ImVec2(12, 8);
	style.ItemSpacing = ImVec2(8, 8);
	style.ItemInnerSpacing = ImVec2(8, 6);

	const float o = opacity;                 // opacidad del “cristal”
	const ImVec4 txt = ImVec4(1, 1, 1, 0.95f);     // texto claro
	const ImVec4 pane = ImVec4(0.16f, 0.16f, 0.18f, o); // panel “vidrioso” oscuro
	const ImVec4 paneHi = ImVec4(0.20f, 0.20f, 0.22f, o);
	const ImVec4 paneLo = ImVec4(0.13f, 0.13f, 0.15f, o * 0.85f);

	// Colores base “glass”
	colors[ImGuiCol_Text] = txt;
	colors[ImGuiCol_TextDisabled] = ImVec4(1, 1, 1, 0.45f);
	colors[ImGuiCol_WindowBg] = pane;     // importante: con alpha
	colors[ImGuiCol_ChildBg] = paneLo;
	colors[ImGuiCol_PopupBg] = paneHi;
	colors[ImGuiCol_Border] = ImVec4(1, 1, 1, 0.10f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0.0f);

	colors[ImGuiCol_FrameBg] = paneLo;
	colors[ImGuiCol_FrameBgHovered] = pane;
	colors[ImGuiCol_FrameBgActive] = paneHi;

	colors[ImGuiCol_TitleBg] = pane;
	colors[ImGuiCol_TitleBgActive] = paneHi;
	colors[ImGuiCol_TitleBgCollapsed] = paneLo;

	colors[ImGuiCol_MenuBarBg] = pane;

	colors[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0.0f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(1, 1, 1, 0.10f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1, 1, 1, 0.18f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1, 1, 1, 0.26f);

	// Acento tipo macOS (azul #0A84FF por defecto)
	colors[ImGuiCol_CheckMark] = accent;
	colors[ImGuiCol_SliderGrab] = accent;
	colors[ImGuiCol_SliderGrabActive] = ImVec4(accent.x, accent.y, accent.z, 1.0f);

	colors[ImGuiCol_Button] = paneLo;
	colors[ImGuiCol_ButtonHovered] = pane;
	colors[ImGuiCol_ButtonActive] = paneHi;

	colors[ImGuiCol_Header] = paneLo;
	colors[ImGuiCol_HeaderHovered] = pane;
	colors[ImGuiCol_HeaderActive] = paneHi;

	colors[ImGuiCol_Separator] = ImVec4(1, 1, 1, 0.10f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(1, 1, 1, 0.18f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(1, 1, 1, 0.30f);

	colors[ImGuiCol_Tab] = paneLo;
	colors[ImGuiCol_TabHovered] = pane;
	colors[ImGuiCol_TabActive] = paneHi;
	colors[ImGuiCol_TabUnfocused] = paneLo;
	colors[ImGuiCol_TabUnfocusedActive] = pane;

	colors[ImGuiCol_DockingPreview] = ImVec4(accent.x, accent.y, accent.z, 0.35f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0, 0, 0, 0.0f);

	colors[ImGuiCol_TableHeaderBg] = pane;
	colors[ImGuiCol_TableBorderStrong] = ImVec4(1, 1, 1, 0.08f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(1, 1, 1, 0.04f);
	colors[ImGuiCol_TableRowBg] = ImVec4(1, 1, 1, 0.03f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1, 1, 1, 0.06f);

	colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.35f);
	colors[ImGuiCol_NavHighlight] = ImVec4(accent.x, accent.y, accent.z, 0.50f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1, 1, 1, 0.30f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0, 0, 0, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0, 0, 0, 0.35f);
}


void
GUI::ToolBar() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {
				// Acción para "New"
			}
			if (ImGui::MenuItem("Open")) {
				// Acción para "Open"
			}
			if (ImGui::MenuItem("Save")) {
				// Acción para "Save"
			}
			if (ImGui::MenuItem("Exit")) {
				// Acción para "Exit"
				show_exit_popup = true;
				ImGui::OpenPopup("Exit?");
				//closeApp();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Undo")) {
				// Acción para "Undo"
			}
			if (ImGui::MenuItem("Redo")) {
				// Acción para "Redo"
			}
			if (ImGui::MenuItem("Cut")) {
				// Acción para "Cut"
			}
			if (ImGui::MenuItem("Copy")) {
				// Acción para "Copy"
			}
			if (ImGui::MenuItem("Paste")) {
				// Acción para "Paste"
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Tools")) {
			if (ImGui::MenuItem("Options")) {
				// Acción para "Options"
			}
			if (ImGui::MenuItem("Settings")) {
				// Acción para "Settings"
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void
GUI::closeApp() {
	if (show_exit_popup) {
		ImGui::OpenPopup("Exit?");
		show_exit_popup = false; // Reset the flag
	}
	// Centrar el popup en la pantalla
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Exit?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Estas a punto de salir de la aplicacion.\nEstas seguro?\n\n");
		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			exit(0); // Salir de la aplicación
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void
GUI::inspectorGeneral(EU::TSharedPointer<Actor> actor) {
	ImGui::Begin("Inspector");
	// Checkbox para Static
	bool isStatic = false;
	ImGui::Checkbox("##Static", &isStatic);
	ImGui::SameLine();

	// Input text para el nombre del objeto
	char objectName[128] = "Cube";
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.6f);
	ImGui::InputText("##ObjectName", &actor->getName()[0], IM_ARRAYSIZE(objectName));
	ImGui::SameLine();

	// Icono (este puede ser una imagen, aquí solo como ejemplo de botón)
	if (ImGui::Button("Icon")) {
		// Lógica del botón de icono aquí
	}

	// Separador horizontal
	ImGui::Separator();

	// Dropdown para Tag
	const char* tags[] = { "Untagged", "Player", "Enemy", "Environment" };
	static int currentTag = 0;
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
	ImGui::Combo("Tag", &currentTag, tags, IM_ARRAYSIZE(tags));
	ImGui::SameLine();

	// Dropdown para Layer
	const char* layers[] = { "Default", "TransparentFX", "Ignore Raycast", "Water", "UI" };
	static int currentLayer = 0;
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
	ImGui::Combo("Layer", &currentLayer, layers, IM_ARRAYSIZE(layers));

	ImGui::Separator();
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		inspectorContainer(actor);
	}
	ImGui::End();
}

void
GUI::inspectorContainer(EU::TSharedPointer<Actor> actor) {
	//ImGui::Begin("Transform");
	// Draw the structure
	vec3Control("Position", const_cast<float*>(actor->getComponent<Transform>()->getPosition().data()));
	vec3Control("Rotation", const_cast<float*>(actor->getComponent<Transform>()->getRotation().data()));
	vec3Control("Scale", const_cast<float*>(actor->getComponent<Transform>()->getScale().data()));

	//ImGui::End();
}

void
GUI::outliner(const std::vector<EU::TSharedPointer<Actor>>& actors) {
	ImGui::Begin("Hierarchy");

	// Barra de búsqueda
	static ImGuiTextFilter filter;
	filter.Draw("Search...", 180.0f); // Barra de búsqueda con ancho ajustable

	ImGui::Separator();

	// Recorrer y mostrar cada actor que pase el filtro de búsqueda
	for (int i = 0; i < actors.size(); ++i) {
		const auto& actor = actors[i];

		// Obtener el nombre del actor o asignar un nombre genérico
		std::string actorName = actor ? actor->getName() : "Actor";

		// Verificar si el actor pasa el filtro de búsqueda
		if (!filter.PassFilter(actorName.c_str())) {
			continue; // Saltar actores que no coincidan con el filtro
		}

		// Si el actor es seleccionable
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (selectedActorIndex == i)
			flags |= ImGuiTreeNodeFlags_Selected;

		// Crear un nodo de árbol para cada actor
		bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)i, flags, "%s", actorName.c_str());

		// Selección de actor
		if (ImGui::IsItemClicked()) {
			selectedActorIndex = i;
			// Aquí puedes llamar a alguna función para mostrar los detalles del actor en otra ventana
		}

		// Mostrar nodos hijos si el nodo está abierto
		if (nodeOpen) {
			ImGui::Text("Position: %.2f, %.2f, %.2f",
				actor->getComponent<Transform>().get()->getPosition().x,
				actor->getComponent<Transform>().get()->getPosition().y,
				actor->getComponent<Transform>().get()->getPosition().z);
			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void
GUI::editTransform(const XMMATRIX& view, const XMMATRIX& projection, EU::TSharedPointer<Actor> actor)
{
	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
	auto transform = actor->getComponent<Transform>();

	// 1) OBTENER COMPONENTES (Asegúrate de que sean float[3])
	float* pos = const_cast<float*>(transform->getPosition().data());
	float* rot = const_cast<float*>(transform->getRotation().data());
	float* sca = const_cast<float*>(transform->getScale().data());

	// 2) CREAR MATRIZ PARA IMGUIZMO 
	// Importante: No uses la matriz de DirectX aquí. 
	// Deja que ImGuizmo cree su propia matriz temporal en su formato preferido.
	float mArr[16];
	ImGuizmo::RecomposeMatrixFromComponents(pos, rot, sca, mArr);

	// 3) PREPARAR MATRICES DE CÁMARA (Transponer para que ImGuizmo las entienda)
	float vArr[16], pArr[16];
	// Probar SIN transponer primero
	ToFloatArray(view, vArr);
	ToFloatArray(projection, pArr);

	// 5) DIBUJAR GIZMO
	ImGuizmo::SetID(0);
	ImGuizmo::SetGizmoSizeClipSpace(0.15f);
	ImGuizmo::AllowAxisFlip(false);

	// Define cuánto quieres que "salte" la rotación (ejemplo: 15 grados)
	float snapValue = 25.0f;
	if (mCurrentGizmoOperation == ImGuizmo::ROTATE) snapValue = 5.0f;
	if (mCurrentGizmoOperation == ImGuizmo::TRANSLATE) snapValue = 0.5f;

	// Crea un array de snap
	float snap[3] = { snapValue, snapValue, snapValue };

	// Usa la versión de Manipulate que acepta snap (es el último parámetro)
	// Si mantienes presionada una tecla (ej. CTRL), aplicas el snap
	bool useSnap = ImGui::GetIO().KeyCtrl;

	//ImGuizmo::Manipulate(
	//	vArr, pArr,
	//	mCurrentGizmoOperation,
	//	mCurrentGizmoMode,
	//	mArr,
	//	NULL,
	//	useSnap ? snap : NULL // Aquí pasas el snap si se desea
	//);

	ImGuizmo::Manipulate(vArr, pArr, mCurrentGizmoOperation, mCurrentGizmoMode, mArr);

	// 6) SI SE ESTÁ USANDO, ACTUALIZAR ACTOR
	if (ImGuizmo::IsUsing()) {
		float newPos[3], newRot[3], newSca[3];

		// Sacamos los datos de la matriz de ImGuizmo
		ImGuizmo::DecomposeMatrixToComponents(mArr, newPos, newRot, newSca);

		// Aplicamos a los componentes del actor
		transform->setPosition(EU::Vector3(newPos[0], newPos[1], newPos[2]));
		transform->setRotation(EU::Vector3(newRot[0], newRot[1], newRot[2]));
		transform->setScale(EU::Vector3(newSca[0], newSca[1], newSca[2]));

		// Sincronizamos la matriz final de DirectX para el renderizado
		// Nota: SR T (Scale * Rotation * Translation)
		XMMATRIX matScale = XMMatrixScaling(newSca[0], newSca[1], newSca[2]);
		XMMATRIX matRot = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(newRot[0]),
			XMConvertToRadians(newRot[1]),
			XMConvertToRadians(newRot[2])
		);
		XMMATRIX matTrans = XMMatrixTranslation(newPos[0], newPos[1], newPos[2]);

		// Esta es la matriz que usará tu Vertex Shader
		transform->matrix = matScale * matRot * matTrans;
	}
}

void GUI::drawGizmoToolbar() {
	// 1. Configurar la posición y estilo de la mini ventana
	ImGuiIO& io = ImGui::GetIO();
	float windowWidth = 120.0f; // Ajusta según necesites

	// Ubicarla a 10px de la esquina superior izquierda
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowBgAlpha(0.35f); // Semi-transparente como en motores reales

	// Flags para que no parezca una ventana común
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("GizmoToolBar", nullptr, window_flags)) {

		// Estilo de los botones (más profesional)
		auto buttonMode = [&](const char* label, ImGuizmo::OPERATION op, const char* shortcut) {
			bool isActive = (mCurrentGizmoOperation == op);
			if (isActive) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f)); // Azul si está activo
			}

			if (ImGui::Button(label)) {
				mCurrentGizmoOperation = op;
			}

			// Tooltip para que el usuario vea el acceso rápido
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("%s (%s)", label, shortcut);
			}

			if (isActive) ImGui::PopStyleColor();
			ImGui::SameLine();
			};

		// Botones de la barra
		buttonMode("T", ImGuizmo::TRANSLATE, "W");
		buttonMode("R", ImGuizmo::ROTATE, "E");
		buttonMode("S", ImGuizmo::SCALE, "R");

		// Opcional: Selector de modo Local/Mundo
		static ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;
		if (ImGui::Button(mCurrentGizmoMode == ImGuizmo::WORLD ? "Global" : "Local")) {
			mCurrentGizmoMode = (mCurrentGizmoMode == ImGuizmo::WORLD) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
		}
	}
	ImGui::End();

	// Acceso rápido por teclado
	if (!ImGui::IsAnyItemActive()) {
		//if (ImGui::IsKeyPressed(ImGuiKey_W)) mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		//if (ImGui::IsKeyPressed(ImGuiKey_E)) mCurrentGizmoOperation = ImGuizmo::ROTATE;
		//if (ImGui::IsKeyPressed(ImGuiKey_R)) mCurrentGizmoOperation = ImGuizmo::SCALE;
	}
}