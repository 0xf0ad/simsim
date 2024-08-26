#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "components.h"
#include "libs/imgui/imgui.h"
#include "libs/imgui/backends/imgui_impl_glfw.h"
#include "libs/imgui/backends/imgui_impl_opengl3.h"
#include "libs/imgui/imgui_internal.h"
#include "libs/glad/glad.h"
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include "ed.h"

#define MSAA_LVL     8
#define WIN_WIDTH 1280
#define WIN_HEIGHT 720

#define max(a, b) a > b ? a : b

// ik global variables are bad but im not awware of any other methode as simple as this
editor_t editor;

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
	editor.aspectratio = (float)width / (float)height;
	editor.resol[0] = width, editor.resol[1] = height;
}

void spawnlink(editor_t* p_editor, pin_t* pin0, pin_t* pin1){
	if(pin0 != pin1){
		if(!pin0->connected_node){
			if(!pin1->connected_node){
				node_t* new_node = add_node(p_editor->head);
				pin1->connected_node = new_node;
			}
			pin0->connected_node = pin1->connected_node;
		} else {
			if(!pin1->connected_node){
				pin1->connected_node = pin0->connected_node;
			} else {
				merge_nodes(p_editor, pin1->connected_node, pin0->connected_node);
			}
		}
		p_editor->links.push_back({.pins = {pin0, pin1}});
	}
}

void checklinks(editor_t* p_editor, double xpos, double ypos, bool mousebuttonpressed){

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos){
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
	// we use the segmoid function as x its input and yoffset as dx/dt and scale as output
	static const double maxscale = 4.l, sensitivity = 20.l;
	//initial value of x, the solution of f(x) = 1
	static double x = -log(maxscale - 1.l);
	double mousepos_towindow[2], mousepos_tocanvas[2];
	glfwGetCursorPos(window, &mousepos_towindow[0], &mousepos_towindow[1]);
	mousepos_tocanvas[0] = (mousepos_towindow[0] - editor.grid.offset[0]) / editor.grid.scale;
	mousepos_tocanvas[1] = (mousepos_towindow[1] - editor.grid.offset[1]) / editor.grid.scale;

	x += yoffset / sensitivity;
	editor.grid.scale = maxscale / (1.l + exp(-x));

	editor.grid.offset[0] = mousepos_towindow[0] - (mousepos_tocanvas[0] * editor.grid.scale);
	editor.grid.offset[1] = mousepos_towindow[1] - (mousepos_tocanvas[1] * editor.grid.scale);
}

void spawn_comp(editor_t* p_editor, double posx, double posy, component_type comp_type){
	// maxlen is lenth of "xnor" and "nand" wich is 4
	size_t maxlen = strlen("textures/components/.png") + strlen(components[comp_type].name) + 1;
	char path[maxlen+1];
	snprintf(path, maxlen,"textures/components/%s.png", components[comp_type].name);

	int weight, height;
	// convert GLuint to ImTexture through void*
	void* texture = (void*)(intptr_t)load_texture(path, &weight, &height);

	component_t comp = {
		.id = get_num_elements(p_editor, comp_type),
		.defenition = components[comp_type],
		.caracteristic = 1.l,
		.pins  = {{.pos = {0, 0}, .connected_node = NULL, .selected = false},
		          {.pos = {0, 0}, .connected_node = NULL, .selected = false},
		          {.pos = {0, 0}, .connected_node = NULL, .selected = false},
		          {.pos = {0, 0}, .connected_node = NULL, .selected = false}},
		.L1 = NULL,
		.L2 = NULL,
		.quad = {
			.texID = texture,
			.rot = 0.l,
			.dims = {(double)weight, (double)height},
			.pos = {posx, posy}
		},
	};

	if(comp_type == coupled_inductors){
		comp.L1 = (component_t*) malloc(sizeof(component_t));
		comp.L2 = (component_t*) malloc(sizeof(component_t));
		comp.L1->id = get_num_elements(p_editor, inductor);
		comp.L2->id = get_num_elements(p_editor, inductor);
		comp.L1->defenition = components[inductor];
		comp.L2->defenition = components[inductor];
		comp.L1->caracteristic = 1.l;
		comp.L2->caracteristic = 1.l;
	}

	p_editor->components.push_back(comp);

}

void recenter_grid(editor_t* p_editor){
	p_editor->grid.offset[0] = p_editor->resol[0] / 2.l;
	p_editor->grid.offset[1] = p_editor->resol[1] / 2.l;
}

void showeditormenu(editor_t* p_editor){
	if(ImGui::BeginPopup("editor menu")){
		ImVec2 mousepos = ImGui::GetMousePosOnOpeningCurrentPopup();
		if(ImGui::BeginMenu("add a component")){
			for(uint32_t i = 1; i < sizeof(components)/sizeof(components[0]); i++)
				if(ImGui::MenuItem(components[i].name))
					spawn_comp(p_editor,
					          (mousepos.x - editor.grid.offset[0]) / editor.grid.scale,
					          (mousepos.y - editor.grid.offset[1]) / editor.grid.scale,
					          components[i].type);
			ImGui::EndMenu();
		}
		if(ImGui::MenuItem("re-center the grid"))
			recenter_grid(p_editor);
		ImGui::EndPopup();
	}
}

void show_comp_menu(editor_t* editor){
	static ImGuiID popupID = 0;
	if(ImGui::BeginPopup("component menu")){
		if(ImGui::MenuItem("rotate by  90 deg"))
			for(auto comp : editor->selected_components)
				comp->quad.rot += 1.570755; // PI/2
		if(ImGui::MenuItem("rotate by -90 deg"))
			for(auto comp : editor->selected_components)
				comp->quad.rot -= 1.570755; // PI/2

		if(editor->selected_components.size() == 1){
			if(ImGui::MenuItem("properies")){
				// a work around cuz I cant open a popup from a menu
				popupID = ImHashStr("comp properties");
				ImGui::PushOverrideID(popupID);
				ImGui::OpenPopup("comp properties");
				ImGui::PopID();
			}
		}
		ImGui::EndPopup();
	}

	if(popupID){
		ImGui::PushOverrideID(popupID);
		if(ImGui::BeginPopupModal("comp properties")){
			ImGui::Text("component settings");
			const char* value_name[] = {"", "resestance", "inductance", "capacitance", "k", "voltage",
			"current", "gain", "gain", "gain", "gain", ""};
			ImGui::InputDouble(value_name[editor->selected_components[0]->defenition.type], &editor->selected_components[0]->caracteristic);
			if(editor->selected_components[0]->defenition.type == curr_cont_curr_source ||
			   editor->selected_components[0]->defenition.type == curr_cont_volt_source){
				static const char* currently_selected = NULL;
				if(ImGui::BeginCombo("contoller", currently_selected)){
					for(uint64_t i = 0; i < editor->components.size(); i++){
						if(editor->components[i].defenition.type == indp_current_source){
							char name[32];
							snprintf(name, 31, "%s %ld", editor->components[i].defenition.name, editor->components[i].id);
							bool is_selected = currently_selected ? (!strcmp(currently_selected, name)) : false;
							if(ImGui::Selectable(name, is_selected)){
								editor->selected_components[0]->Vcont = &editor->components[i];
								if(currently_selected) free((void*) currently_selected);
								currently_selected = strdup(name);
							}
							if(is_selected) ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			} else if(editor->selected_components[0]->defenition.type == coupled_inductors){
				ImGui::InputDouble("inductance 1", &editor->selected_components[0]->L1->caracteristic);
				ImGui::InputDouble("inductance 2", &editor->selected_components[0]->L2->caracteristic);
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}
}

void processInput(GLFWwindow* window, editor_t* editor){
	static double lastmousepos[2];
	static bool draged = false;
	static bool rightpressed = false;
	static bool pin_was_selected = false;
	//static std::vector<double[2]> joints;
	double minimum = 10;
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)){
		for(uint64_t i = 0; i < editor->components.size(); i++){
			bool mouseovernode = false;
			for(uint64_t j = 0; j < editor->components[i].defenition.num_pins; j++){
				double diffx = x - editor->components[i].pins[j].pos[0];
				double diffy = y - editor->components[i].pins[j].pos[1];
				if((diffx*diffx + diffy*diffy) < minimum * minimum && !pin_was_selected){
					editor->components[i].pins[j].selected = true;
					editor->connector = &editor->components[i].pins[j];
					pin_was_selected = true;
					mouseovernode = true;
				}
			}
			// TODO :: you should grab it slowlly
			if(mouse_over_quad(editor, x, y, &editor->components[i].quad) && (!mouseovernode && !editor->connector)){
				// select component
				editor->components[i].quad.pos[0] += x - lastmousepos[0],
				editor->components[i].quad.pos[1] += y - lastmousepos[1];
				break;
			}
		}
	}if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE){
		if(pin_was_selected){
			for(uint64_t i = 0; i < editor->components.size(); i++){
				for(uint64_t j = 0; j < editor->components[i].defenition.num_pins; j++){
					double diffx = x - editor->components[i].pins[j].pos[0];
					double diffy = y - editor->components[i].pins[j].pos[1];
					if((diffx*diffx + diffy*diffy) < minimum * minimum)
						spawnlink(editor, editor->connector, &editor->components[i].pins[j]);
				}
			}
			pin_was_selected = false;
			editor->connector->selected = false;
			editor->connector = NULL;
		}
	} if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)){
		if(lastmousepos[0] != x || lastmousepos[1] != y){
			editor->grid.offset[0] += x - lastmousepos[0],
			editor->grid.offset[1] += y - lastmousepos[1];
			draged = true;
		}
		rightpressed = true;
	} if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE){
		if(!draged && rightpressed){
			rightpressed = false;
			for(uint64_t i = 0; i < editor->components.size(); i++){
				if(mouse_over_quad(editor, x, y, &editor->components[i].quad)){
					ImGui::OpenPopup("component menu");
					editor->selected_components = {&editor->components[i]};
					goto fuckoff;
				}
			}
			ImGui::OpenPopup("editor menu");
		} else if(rightpressed){
			draged = rightpressed = false;
		}
		fuckoff: ;
	}
	lastmousepos[0] = x, lastmousepos[1] = y;
};

void Menu(){
	static ImGuiID popupID = 0;
	if (ImGui::BeginMenuBar()){
		if (ImGui::BeginMenu("File")){
			if (ImGui::MenuItem("New", nullptr, false, true)){}
			if (ImGui::MenuItem("Open", nullptr, false, true)){}
			ImGui::Separator();
			if (ImGui::MenuItem("Save", nullptr, false, true)){}
			if (ImGui::MenuItem("Save As...", nullptr, false, true)){}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", nullptr, false, true)){}
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("solve circuit")){
			if(ImGui::MenuItem("solve for G matrix")){
				constract_matrices(&editor);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help")){
			if (ImGui::MenuItem("About", nullptr, false, true)){
				popupID = ImHashStr("about window");
				ImGui::PushOverrideID(popupID);
				ImGui::OpenPopup( "about window" );
				ImGui::PopID();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	// work around not being able to init popup wondows from menu items
	if(popupID){
		ImGui::PushOverrideID(popupID);
		if(ImGui::BeginPopupModal("about window")){
			ImGui::Text("dont u know me");
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}
}

void drawgrid(ImDrawList* drawlist){

	const float lgrid = editor.grid.step * editor.grid.scale;
	int markX = editor.grid.offset[0] / lgrid,
	    markY = editor.grid.offset[1] / lgrid;

	// TODO: edit the function to draw only inside the canvas
	drawlist->AddQuadFilled(ImVec2(editor.grid.offset[0] - 10, editor.grid.offset[1] + 10),
	                        ImVec2(editor.grid.offset[0] + 10, editor.grid.offset[1] + 10),
	                        ImVec2(editor.grid.offset[0] + 10, editor.grid.offset[1] - 10),
	                        ImVec2(editor.grid.offset[0] - 10, editor.grid.offset[1] - 10),
	                        ImColor(255, 0, 0, 255));
	for(float x = fmodf(editor.grid.offset[0], lgrid); x < editor.resol[0]; x += lgrid, markX--)
		drawlist->AddLine(ImVec2(x, 0.f), ImVec2(x, editor.resol[1]),
		                  markX % 8 ? IM_COL32(100, 100, 100, 128) : IM_COL32(0, 0, 0, 255));
	for(float y = fmodf(editor.grid.offset[1], lgrid); y < editor.resol[1]; y += lgrid, markY--)
		drawlist->AddLine(ImVec2(0.f, y), ImVec2(editor.resol[0], y),
		                  markY % 8 ? IM_COL32(100, 100, 100, 128) : IM_COL32(0, 0, 0, 255));
}

void draw_comps(ImDrawList* drawlist){
	for(uint32_t i = 0; i != editor.components.size(); i++){
		const Quad_t* q = &editor.components[i].quad;
		const double cosr = cos(q->rot), sinr = sin(q->rot);

		ImVec2 p[4];
		p[0] = ImVec2(editor.grid.offset[0] + ((q->pos[0] - (((q->dims[0] * cosr) + (q->dims[1] * sinr)) / 4.l)) * editor.grid.scale),
		              editor.grid.offset[1] + ((q->pos[1] + (((q->dims[1] * cosr) - (q->dims[0] * sinr)) / 4.l)) * editor.grid.scale));
		p[1] = ImVec2(editor.grid.offset[0] + ((q->pos[0] + (((q->dims[0] * cosr) - (q->dims[1] * sinr)) / 4.l)) * editor.grid.scale),
		              editor.grid.offset[1] + ((q->pos[1] + (((q->dims[1] * cosr) + (q->dims[0] * sinr)) / 4.l)) * editor.grid.scale));
		p[2] = ImVec2(editor.grid.offset[0] + ((q->pos[0] + (((q->dims[0] * cosr) + (q->dims[1] * sinr)) / 4.l)) * editor.grid.scale),
		              editor.grid.offset[1] + ((q->pos[1] - (((q->dims[1] * cosr) - (q->dims[0] * sinr)) / 4.l)) * editor.grid.scale));
		p[3] = ImVec2(editor.grid.offset[0] + ((q->pos[0] - (((q->dims[0] * cosr) - (q->dims[1] * sinr)) / 4.l)) * editor.grid.scale),
		              editor.grid.offset[1] + ((q->pos[1] - (((q->dims[1] * cosr) + (q->dims[0] * sinr)) / 4.l)) * editor.grid.scale));

		drawlist->AddImageQuad(q->texID, p[0], p[1], p[2], p[3]);

		ImVec2 npos;
		for(uint64_t j = 0; j < editor.components[i].defenition.num_pins; j++){
			uint8_t indices[] = {0, 1, 3, 2};
			uint8_t a = indices[j%2], b = indices[(j%2) + 2];
			if(editor.components[i].defenition.num_pins == 2)
				npos = ImVec2((p[a].x + p[b].x) / 2, (p[a].y + p[b].y) / 2);
			else if(editor.components[i].defenition.num_pins == 3)
				if(j == 1)
					npos = ImVec2((p[a].x + p[b].x) / 2,(p[a].y + p[b].y) / 2);
				else
					npos = ImVec2((p[a].x + p[b].x) / 2, (((j >> 1) + 1) * (p[a].y - p[b].y) / 3) + p[b].y);
			else if(editor.components[i].defenition.num_pins == 4)
					npos = ImVec2((((j >> 1) + 1) * (p[a].x - p[b].x) / 3) + p[b].x,
				                  (((j >> 1) + 1) * (p[a].y - p[b].y) / 3) + p[b].y);
			editor.components[i].pins[j].pos[0] = npos.x, editor.components[i].pins[j].pos[1] = npos.y;
			ImColor color = IM_COL32(0, 0, 0, 255);
			if(editor.components[i].pins[j].selected){
				color = IM_COL32(0, 255, 0, 255);
			}
			drawlist->AddCircleFilled(npos, 7.5 * editor.grid.scale, color);
		}
	}
}

void drawlinks(ImDrawList* drawlist){
	for(size_t i = 0; i < editor.links.size(); i++){
		drawlist->AddLine(ImVec2(editor.links[i].pins[0]->pos[0], editor.links[i].pins[0]->pos[1]),
		                  ImVec2(editor.links[i].pins[1]->pos[0], editor.links[i].pins[1]->pos[1]),
		                 IM_COL32(0, 0, 0, 255));
	}
	if(editor.connector){
		drawlist->AddLine(ImVec2(editor.connector->pos[0], editor.connector->pos[1]),
		                  ImGui::GetMousePos(), IM_COL32(255, 0, 0, 255));
	}
}

void Dockspace(){
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.0f);

	#define windowflags ImGuiWindowFlags_MenuBar              |\
	                    ImGuiWindowFlags_NoDocking            |\
	                    ImGuiWindowFlags_NoTitleBar           |\
	                    ImGuiWindowFlags_NoCollapse           |\
	                    ImGuiWindowFlags_NoResize             |\
	                    ImGuiWindowFlags_NoMove               |\
	                    ImGuiWindowFlags_NoBringToFrontOnFocus|\
	                    ImGuiWindowFlags_NoNavFocus


	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	static bool openDockspace;
	ImGui::Begin("DockSpace", &openDockspace, windowflags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	Menu();
	ImGui::End();

	ImGui::Begin("Simulation", nullptr, ImGuiWindowFlags_None);
	ImGui::Text("Text");
	ImGui::Text("(%.1f FPS)", ImGui::GetIO().Framerate);
	ImGui::End();

	ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_None);
	if(editor.selected_components.size()){
		for(auto compo : editor.selected_components){
			ImGui::Text("%s %lu", compo->defenition.name, compo->id);
			ImGui::SliderAngle("angle", &compo->quad.rot);
			const char* name[] = {"", "resestance", "inductance", "capacitance", "k", "voltage",
			"current", "gain", "gain", "gain", "gain", ""};
			ImGui::InputDouble(name[compo->defenition.type], &compo->caracteristic);
			ImGui::Separator();
		}
	}
	ImGui::End();

	ImGui::Begin("Explorer", nullptr, ImGuiWindowFlags_None);
	ImGui::Text("numnoded : %lu", get_num_nodes(&editor));
	for(size_t i = 0; i < editor.components.size(); i++)
		ImGui::Text("id: %s\tfrom: %lu to: %lu", editor.components[i].defenition.name, editor.components[i].pins[0].connected_node ? editor.components[i].pins[0].connected_node->id : -1, editor.components[i].pins[1].connected_node ? editor.components[i].pins[1].connected_node->id : -1);
	ImGui::End();

	ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_None);
	ImGui::End();

	ImGui::Begin("Diagram", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImDrawList* drawlist = ImGui::GetWindowDrawList();
	drawgrid(drawlist);
	draw_comps(drawlist);
	drawlinks(drawlist);
	ImGui::End();
}

int main(void){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, MSAA_LVL);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPACT, GL_TRUE);
#endif

	if(!glfwInit()){
		fprintf(stderr, "couldnt initialize glfw\n");
		return EXIT_FAILURE;
	}

	GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "sim or smthing idk", NULL, NULL);
	if(!window){
		fprintf(stderr, "failed to create window\n");
		glfwTerminate();
		return EXIT_FAILURE;
	}

	editor = {
		.aspectratio = ((float)WIN_WIDTH / (float)WIN_HEIGHT),
		.num_indp_vol_sources = 0,
		.resol = {WIN_WIDTH, WIN_HEIGHT},
		.grid = {
			.scale = 1.l,
			.step = 64.l,
			.offset = {0.l, 0.l},
		},
		.head = add_node(NULL),
		.connector = NULL,
		.components = std::vector<component_t>(),
		.links = std::vector<link_t>(),
		.selected_components = std::vector<component_t*>(),
		.selected_links = std::vector<link_t*>()
	};

	glfwMakeContextCurrent(window);
	gladLoadGL();
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		fprintf(stderr, "couldnt initialize glad\n");
		glfwTerminate();
		return EXIT_FAILURE;
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);
	glfwSwapInterval(1);

	recenter_grid(&editor);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsLight();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450 core");
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigDockingWithShift = true;
	io.ConfigWindowsResizeFromEdges = true;
	io.IniFilename = "layout.ini";
	float bgcolor[3] = {.2, .3, .3};
	double mousex, mousey;

	while(!glfwWindowShouldClose(window)){

		glfwGetCursorPos(window, &mousex, &mousey);

		glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], 1.);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		Dockspace();
		process(&editor);

		ImGui::ShowDemoWindow();
		processInput(window, &editor);
		showeditormenu(&editor);
		show_comp_menu(&editor);


		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return EXIT_SUCCESS;
}
