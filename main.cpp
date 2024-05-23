#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include "components.h"
#include "libs/imgui/imgui.h"
#include "libs/imgui/backends/imgui_impl_glfw.h"
#include "libs/imgui/backends/imgui_impl_opengl3.h"
#include "libs/glad/glad.h"
#include <GLFW/glfw3.h>
#include <vector>
#include "ed.h"

#define MSAA_LVL     8
#define WIN_WIDTH 1280
#define WIN_HEIGHT 720

editor_t editor;

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
	editor.aspectratio = (float)width / (float)height;
	editor.resol[0] = width, editor.resol[1] = height;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn){
	//for(uint64_t i = 0; i < editor.nodescount; i++){
	//}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
	// we use the segmoid function as x its input and yoffset as dx/dt and scale as output
	static const double maxscale = 4.l, sensitivity = 20.l;
	static double x = -log(maxscale - 1.l);
	double Mx, My;
	//static double pMx = 0.l, pMy = 0.l;
	//static double prevscale = editor.grid.scale;
	glfwGetCursorPos(window, &Mx, &My);
	x += yoffset / sensitivity;
	double prevoffset[2] = {editor.grid.offset[0], editor.grid.offset[1]};
	double mousepos_tocanvasx = (Mx - editor.grid.offset[0]) / editor.grid.scale;
	double mousepos_tocanvasy = (My - editor.grid.offset[1]) / editor.grid.scale;
	editor.grid.scale = maxscale / (1.l + exp(-x));
	editor.grid.offset[0] = Mx - (mousepos_tocanvasx * editor.grid.scale);
	editor.grid.offset[1] = My - (mousepos_tocanvasy * editor.grid.scale);
}

void spawnnode(editor_t* editor, double posx, double posy, logicgatetype_t logictype){

	static uint64_t ID = 0;

	const char* texturepath[] = {"undef", "buf", "not", "and", "nand", "or", "nor", "xor", "xnor"};
	// maxlen is lenth of "xnor" and "nand" wich is 4
	size_t maxlen = sizeof("textures/logic/.png") + sizeof(texturepath[logictype]);
	char path[maxlen+1];
	snprintf(path, maxlen,"textures/logic/%s.png", texturepath[logictype]);

	int w, h;
	// convert GLuint to ImTexture through void*
	void* texture = (void*)(intptr_t)load_texture(path, &w, &h);
	editor->nodes.push_back({
		.id = ID++,
		.in_pins = NULL,
		.out_pins = NULL,
		.quad = {texture,
			{(double)w, (double)h},
			{posx, posy}
			},
		.selected = false
		}
	);

}

void spawnlink(double x, double y){

}

void recenter_grid(editor_t* editor){
	editor->grid.offset[0] = editor->resol[0] / 2.l;
	editor->grid.offset[1] = editor->resol[1] / 2.l;
}

void showeditormenu(){
	logicgatetype_t selectedtype = undeffined;
	if(ImGui::BeginPopup("editor menu")){
		ImVec2 mousepos = ImGui::GetMousePosOnOpeningCurrentPopup();
		if(ImGui::BeginMenu("add a logic gate")){
			if(ImGui::MenuItem("buffer")) selectedtype = buffergate;
			if(ImGui::MenuItem("not"))    selectedtype = notgate;
			if(ImGui::MenuItem("and"))    selectedtype = andgate;
			if(ImGui::MenuItem("nand"))   selectedtype = nandgate;
			if(ImGui::MenuItem("or"))     selectedtype = orgate;
			if(ImGui::MenuItem("nor"))    selectedtype = norgate;
			if(ImGui::MenuItem("xor"))    selectedtype = xorgate;
			if(ImGui::MenuItem("xnor"))   selectedtype = xnorgate;
			ImGui::EndMenu();
			if(selectedtype != undeffined)
				spawnnode(&editor,
				          (mousepos.x - editor.grid.offset[0]) / editor.grid.scale,
				          (mousepos.y - editor.grid.offset[1]) / editor.grid.scale,
				          selectedtype);
		}
		if(ImGui::MenuItem("re-center the grid"))
			recenter_grid(&editor);
		ImGui::EndPopup();
	}
}

void processInput(GLFWwindow* window, editor_t* editor){
	double x, y;
	static double lastmousepos[2];
	static bool draged = false;
	static bool rightpressed = false;
	glfwGetCursorPos(window, &x, &y);

	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)){
		for(uint64_t i = 0; i < editor->nodescount; i++){
			if(mouseovernode(x, y, &editor->nodes[i])){
				editor->nodes[i].selected = true;
				break;
			}
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
			for(uint64_t i = 0; i < editor->nodescount; i++){
				if(mouseovernode(x, y, &editor->nodes[i])){
					show_node_menu(&editor->nodes[i]);
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

		if (ImGui::BeginMenu("View")){
			if (ImGui::MenuItem("Redock", nullptr, false, true)){}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help")){
			if (ImGui::MenuItem("About", nullptr, false, true)){}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void drawgrid(ImDrawList* drawlist){

	const float lgrid = editor.grid.step * editor.grid.scale;
	int markX = editor.grid.offset[0] / lgrid,
	    markY = editor.grid.offset[1] / lgrid;

	// TODO: edit the function to draw only inside the canvas
	drawlist->AddQuadFilled(ImVec2(editor.grid.offset[0]-10, editor.grid.offset[1]+10),
	                        ImVec2(editor.grid.offset[0]+10, editor.grid.offset[1]+10),
	                        ImVec2(editor.grid.offset[0]+10, editor.grid.offset[1]-10),
	                        ImVec2(editor.grid.offset[0]-10, editor.grid.offset[1]-10),
	                        ImColor(255, 0, 0, 255));
	for(float x = fmodf(editor.grid.offset[0], lgrid); x < editor.resol[0]; x += lgrid, markX--)
		drawlist->AddLine(ImVec2(x, 0.f), ImVec2(x, editor.resol[1]), markX % 8 ? IM_COL32(100, 100, 100, 128) : IM_COL32(255, 255, 255, 128));
	for(float y = fmodf(editor.grid.offset[1], lgrid); y < editor.resol[1]; y += lgrid, markY--)
		drawlist->AddLine(ImVec2(0.f, y), ImVec2(editor.resol[0], y), markY % 8 ? IM_COL32(100, 100, 100, 128) : IM_COL32(255, 255, 255, 128));
}

void drawnodes(ImDrawList* drawlist){
	for(uint32_t i = 0; i != editor.nodescount; i++){
		const Quad_t* q = &editor.nodes[i].quad;
		ImVec2 p1 = ImVec2(editor.grid.offset[0] + ((q->pos[0] - (q->dims[0] / 2.l)) * editor.grid.scale),
		                   editor.grid.offset[1] + ((q->pos[1] - (q->dims[1] / 2.l)) * editor.grid.scale));
		ImVec2 p2 = ImVec2(editor.grid.offset[0] + ((q->pos[0] + (q->dims[0] / 2.l)) * editor.grid.scale),
		                   editor.grid.offset[1] + ((q->pos[1] + (q->dims[1] / 2.l)) * editor.grid.scale));

		drawlist->AddImage(q->texID, p1, p2);
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
	ImGui::Text("scale: %f", editor.grid.scale);
	ImGui::End();
	
	ImGui::Begin("Library", nullptr, ImGuiWindowFlags_None);
	ImGui::End();
	
	ImGui::Begin("Explorer", nullptr, ImGuiWindowFlags_None);
	ImGui::End();
	
	ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_None);
	ImGui::End();
	
	ImGui::Begin("Diagram", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::Text("offset: x=%f\ty=%f", editor.grid.offset[0], editor.grid.offset[1]);
	ImGui::Text("mousePos : x= %f\ty=%f", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
	ImGui::Text("Mp : x= %f\ty=%f",  ImGui::GetIO().MousePos.x - editor.grid.offset[0], ImGui::GetIO().MousePos.y - editor.grid.offset[1]);
	drawgrid(ImGui::GetWindowDrawList());
	drawnodes(ImGui::GetWindowDrawList());
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
		.nodescount = 0,
		.resol = {WIN_WIDTH, WIN_HEIGHT},
		.grid = {
			.scale = 1.l,
			.step = 64.l,
			.offset = {0.l, 0.l},
		}
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
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450 core");
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigDockingWithShift = true;
	io.ConfigWindowsResizeFromEdges = true;
	io.IniFilename = "layout.ini";
	float gridstep = 64.f;
	float bgcolor[3] = {.2, .3, .3};

	while(!glfwWindowShouldClose(window)){

		glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], 1.);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		Dockspace();

		//ImGui::ShowMetricsWindow();


		//ImGui::ShowDemoWindow();
		processInput(window, &editor);
		showeditormenu();


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
