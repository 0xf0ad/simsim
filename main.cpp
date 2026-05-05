#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "components.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <vector>
#include "gui.h"

#define MSAA_LVL     8
#define WIN_WIDTH 1280
#define WIN_HEIGHT 720

// ik global variables are bad but im not awware of any other methode as simple as this
editor_t   editor;
app_mode_t  app_mode = MODE_SELECT;

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
	editor.aspectratio = (float)width / (float)height;
	editor.resol[0] = width, editor.resol[1] = height;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
	//if (app_mode != MODE_CIRCUIT) return; // EM zoom is handled inside ImGui
	// we use the segmoid function as x its input and yoffset as dx/dt and scale as output
	static const double maxscale = 4.l, sensitivity = 20.l;
	//initial value of x, the solution of f(x) = 1
	static double x = -log(maxscale - 1.l);
	
	for(size_t i = 0; i < editor.pins.size(); i++){
		// ok this is wrong but it needs to be done for  the second loop to be right
		editor.pins[i]->pos[0] = (editor.pins[i]->pos[0] - editor.grid.offset[0]) / editor.grid.scale;
		editor.pins[i]->pos[1] = (editor.pins[i]->pos[1] - editor.grid.offset[1]) / editor.grid.scale;
	}

	double mousepos_towindow[2], mousepos_tocanvas[2];
	glfwGetCursorPos(window, &mousepos_towindow[0], &mousepos_towindow[1]);
	mousepos_tocanvas[0] = (mousepos_towindow[0] - editor.grid.offset[0]) / editor.grid.scale;
	mousepos_tocanvas[1] = (mousepos_towindow[1] - editor.grid.offset[1]) / editor.grid.scale;

	x += yoffset / sensitivity;
	editor.grid.scale = maxscale / (1.l + exp(-x));

	editor.grid.offset[0] = mousepos_towindow[0] - (mousepos_tocanvas[0] * editor.grid.scale);
	editor.grid.offset[1] = mousepos_towindow[1] - (mousepos_tocanvas[1] * editor.grid.scale);

	for(size_t i = 0; i < editor.pins.size(); i++){
		editor.pins[i]->pos[0] = editor.grid.offset[0] + editor.pins[i]->pos[0] * editor.grid.scale;
		editor.pins[i]->pos[1] = editor.grid.offset[1] + editor.pins[i]->pos[1] * editor.grid.scale;
	}
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
		// we consider the head as the ground point
		.head = add_node(NULL),
		.connector = NULL,
		.components = std::vector<component_t>(),
		.links = std::vector<link_t>(),
		.pins = std::vector<pin_t*>(),
		.selected_components = std::vector<component_t*>(),
		.selected_links = std::vector<link_t*>(),

		.shapes = std::vector<em_shape_t>(),
		.sources = std::vector<em_source_t>(),
		.probes = std::vector<em_probe_t>(),
		.active_material = {
			.conductivity = 5.8e7f,
			.permittivity = 1.0f,
			.permeability = 1.0f,
			.name = "Conductor",
			.color = ImVec4(1.0f, 0.8f, 0.2f, 0.55f),
		},
		.fdtd = {0, 0, 0.f, 0.f, 0.f, 0, {}, {}, {}, {}, {}, {}, false, false, 0.f},
		.draw_mode = EM_SHAPE_RECT,
		.drawing = 0,
		.draw_start = ImVec2(0, 0),
		.poly_wip = std::vector<ImVec2>(),
		.selected_idx = -1,
		.selected_source_idx = -1,
		.selected_probe_idx = -1,
		.panning = 0,
		.pan_last = ImVec2(0, 0),
		.editing_shape = false,
		.edit_handle = -1
	};

	glfwMakeContextCurrent(window);
	gladLoadGL();
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//glfwSetCursorPosCallback(window, mouse_callback);
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
	ImPlot::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
#ifdef LIGHTTHEME
	ImGui::StyleColorsLight();
#else
	ImGui::StyleColorsDark();
#endif
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450 core");
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigDockingWithShift = true;
	io.ConfigWindowsResizeFromEdges = true;
	io.IniFilename = "layout.ini";
#ifdef LIGHTTHEME
	float bgcolor[3] = {0.9f, 0.9f, 0.9f};
#else
	float bgcolor[3] = {0.1f, 0.1f, 0.11f};
#endif
	double mousex, mousey;

	while(!glfwWindowShouldClose(window)){

		glfwGetCursorPos(window, &mousex, &mousey);

		glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], 1.);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (app_mode == MODE_SELECT) {
			show_mode_select(&app_mode);
		} else {
			Dockspace(window, &editor, app_mode);
			// Run FDTD simulation if in EM mode and running
			if (app_mode == MODE_EM && editor.fdtd.running) {
				fdtd_step(&editor);
			}
		}


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
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
	glfwTerminate();
	for(size_t i = 0; i < editor.pins.size(); i++)
		free(editor.pins[i]);
	return EXIT_SUCCESS;
}
