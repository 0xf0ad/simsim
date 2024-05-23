#include <cstddef>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
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

#define max(a, b) a > b ? a : b

// ik global variables are bad but im not awware of any other methode as simple as this
editor_t editor;

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
	editor.aspectratio = (float)width / (float)height;
	editor.resol[0] = width, editor.resol[1] = height;
}

//void mouse_callback(GLFWwindow* window, double xposIn, double yposIn){
	//for(uint64_t i = 0; i < editor.nodescount; i++){
	//}
//}

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

void spawnnode(editor_t* p_editor, double posx, double posy, logicgatetype_t logictype){
	const char* texturepath[] = {"undef", "buf", "not", "and", "nand", "or", "nor", "xor", "xnor"};
	void (*funcs[])(std::vector<pin_t>&, std::vector<pin_t>&) = {NULL,&buffer_transfer, &not_transfer, &and_transfer, &nand_transfer, &or_transfer, &nor_transfer, &xor_transfer, &xnor_transfer};

	// maxlen is lenth of "xnor" and "nand" wich is 4
	size_t maxlen = sizeof("textures/logic/.png") + sizeof(texturepath[logictype]);
	char path[maxlen+1];
	snprintf(path, maxlen,"textures/logic/%s.png", texturepath[logictype]);

	int weight, height;
	// convert GLuint to ImTexture through void*
	void* texture = (void*)(intptr_t)load_texture(path, &weight, &height);
	p_editor->nodes.push_back({
		.id = p_editor->nodescount++,
		.quad = {texture,
			{(double)weight, (double)height},
			{posx, posy}
			},
		.selected = false,
		.in_pins = std::vector<pin_t>({{
			.pos = {posx, posy},
			.value = 0.l,
			.selected = false
		},{
			.pos = {posx, posy},
			.value = 0.l,
			.selected = false
		}}),
		.out_pins = std::vector<pin_t>({{
			.pos = {posx, posy},
			.value = 0.l,
			.selected = false
		}}),
		.H = funcs[logictype]
		}
	);
}

void spawnlink(editor_t* p_editor, pin_t* p_in, pin_t* p_out){
	p_editor->links.push_back({
		.value = 0.l,
		.in = p_in,
		.out = p_out
		}
	);
}

void recenter_grid(editor_t* p_editor){
	p_editor->grid.offset[0] = p_editor->resol[0] / 2.l;
	p_editor->grid.offset[1] = p_editor->resol[1] / 2.l;
}

void showeditormenu(editor_t* p_editor){
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
				spawnnode(p_editor,
				          (mousepos.x - editor.grid.offset[0]) / editor.grid.scale,
				          (mousepos.y - editor.grid.offset[1]) / editor.grid.scale,
				          selectedtype);
		}
		if(ImGui::MenuItem("re-center the grid"))
			recenter_grid(p_editor);
		if(ImGui::MenuItem("SEGFAULT01"))
			spawnlink(&editor, &editor.nodes[0].out_pins[0], &editor.nodes[1].in_pins[0]);
		if(ImGui::MenuItem("SEGFAULT21"))
			spawnlink(&editor, &editor.nodes[2].out_pins[0], &editor.nodes[1].in_pins[1]);
		if(ImGui::MenuItem("SEGFAULT13"))
			spawnlink(&editor, &editor.nodes[1].out_pins[0], &editor.nodes[3].in_pins[1]);
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
					//show_node_menu(&editor->nodes[i]);
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
	drawlist->AddQuadFilled(ImVec2(editor.grid.offset[0] - 10, editor.grid.offset[1] + 10),
	                        ImVec2(editor.grid.offset[0] + 10, editor.grid.offset[1] + 10),
	                        ImVec2(editor.grid.offset[0] + 10, editor.grid.offset[1] - 10),
	                        ImVec2(editor.grid.offset[0] - 10, editor.grid.offset[1] - 10),
	                        ImColor(255, 0, 0, 255));
	for(float x = fmodf(editor.grid.offset[0], lgrid); x < editor.resol[0]; x += lgrid, markX--)
		drawlist->AddLine(ImVec2(x, 0.f), ImVec2(x, editor.resol[1]),
		                  markX % 8 ? IM_COL32(100, 100, 100, 128) : IM_COL32(255, 255, 255, 128));
	for(float y = fmodf(editor.grid.offset[1], lgrid); y < editor.resol[1]; y += lgrid, markY--)
		drawlist->AddLine(ImVec2(0.f, y), ImVec2(editor.resol[0], y),
		                  markY % 8 ? IM_COL32(100, 100, 100, 128) : IM_COL32(255, 255, 255, 128));
}

void drawnodes(ImDrawList* drawlist){
	for(uint32_t i = 0; i != editor.nodescount; i++){
		const Quad_t* q = &editor.nodes[i].quad;
		ImVec2 p1 = ImVec2(editor.grid.offset[0] + ((q->pos[0] - (q->dims[0] / 2.l)) * editor.grid.scale),
		                   editor.grid.offset[1] + ((q->pos[1] - (q->dims[1] / 2.l)) * editor.grid.scale));
		ImVec2 p2 = ImVec2(editor.grid.offset[0] + ((q->pos[0] + (q->dims[0] / 2.l)) * editor.grid.scale),
		                   editor.grid.offset[1] + ((q->pos[1] + (q->dims[1] / 2.l)) * editor.grid.scale));

		drawlist->AddImage(q->texID, p1, p2);
		ImGui::Text("id = %lu\tval = %f", editor.nodes[i].id, editor.nodes[i].out_pins[0].value);
	}
}

void drawlinks(ImDrawList* drawlist){
	for(size_t i = 0; i < editor.links.size(); i++){
		drawlist->AddLine(ImVec2((editor.grid.offset[0] + editor.links[i].in->pos[0]) * editor.grid.scale,
		                            (editor.grid.offset[1] + editor.links[i].in->pos[1]) * editor.grid.scale),
		                 ImVec2((editor.grid.offset[0] + editor.links[i].out->pos[0]) * editor.grid.scale,
		                         (editor.grid.offset[1] + editor.links[i].out->pos[1]) * editor.grid.scale),
		                  editor.links[i].value == 0 ? IM_COL32(255, 0, 0, 255) : IM_COL32(0, 255, 0, 255));
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
	
	ImGui::Begin("Library", nullptr, ImGuiWindowFlags_None);
	ImGui::End();
	
	ImGui::Begin("Explorer", nullptr, ImGuiWindowFlags_None);
	ImGui::End();
	
	ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_None);
	ImGui::End();
	
	ImGui::Begin("Diagram", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImDrawList* drawlist = ImGui::GetWindowDrawList();
	drawgrid(drawlist);
	drawnodes(drawlist);
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
		.nodescount = 0,
		.resol = {WIN_WIDTH, WIN_HEIGHT},
		.grid = {
			.scale = 1.l,
			.step = 64.l,
			.offset = {0.l, 0.l},
		},
		.nodes = std::vector<node_t>(),
		.links = std::vector<link_t>()
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
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450 core");
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigDockingWithShift = true;
	io.ConfigWindowsResizeFromEdges = true;
	io.IniFilename = "layout.ini";
	float bgcolor[3] = {.2, .3, .3};

	while(!glfwWindowShouldClose(window)){

		process(&editor);

		glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], 1.);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		Dockspace();

		//ImGui::ShowMetricsWindow();


		//ImGui::ShowDemoWindow();
		processInput(window, &editor);
		showeditormenu(&editor);


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
