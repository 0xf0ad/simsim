#ifndef GUI_H_
#define GUI_H_

#include "components.h"
#include "matrix.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "implot/implot.h"
#include "ed.h"
#include <GLFW/glfw3.h>

inline void spawn_link(editor_t* p_editor, pin_t* pin0, pin_t* pin1){
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

inline void spawn_comp(editor_t* p_editor, double posx, double posy, component_type comp_type){
	// maxlen is lenth of "xnor" and "nand" wich is 4
	size_t maxlen = strlen("textures/components/.png") + strlen(components[comp_type].name) + 1;
	char path[maxlen+1];
	snprintf(path, maxlen,"textures/components/%s.png", components[comp_type].name);

	int weight, height;
	// convert GLuint to ImTexture through void*
	void* texture = (void*)(intptr_t)load_texture(path, &weight, &height);

	component_t comp = {
		.id = get_num_elements(p_editor, comp_type),
		.definition = components[comp_type],
		.caracteristic = 1.l,
		.pins = (pin_t*) malloc(sizeof(pin_t) * components[comp_type].num_pins),
		.L1 = NULL,
		.L2 = NULL,
		.quad = {
			.texID = texture,
			.rot = 0.l,
			.dims = {(double)weight, (double)height},
			.pos = {posx, posy}
		},
	};

	for(uint8_t i = 0; i < components[comp_type].num_pins; i++){
		comp.pins[i].pos[0] = comp.pins[i].pos[1] = 0;
		comp.pins[i].connected_node = NULL;
		comp.pins[i].selected = false;
	}

	if(comp_type == ground)
		comp.pins[0].connected_node = p_editor->head;

	if(comp_type == coupled_inductors){
		comp.L1 = (component_t*) malloc(sizeof(component_t));
		comp.L2 = (component_t*) malloc(sizeof(component_t));
		comp.L1->id = get_num_elements(p_editor, inductor);
		comp.L2->id = get_num_elements(p_editor, inductor);
		comp.L1->definition = components[inductor];
		comp.L2->definition = components[inductor];
		comp.L1->caracteristic = 1.l;
		comp.L2->caracteristic = 1.l;
	}

	p_editor->components.push_back(comp);
}

inline void recenter_grid(editor_t* p_editor){
	p_editor->grid.offset[0] = p_editor->resol[0] / 2.l;
	p_editor->grid.offset[1] = p_editor->resol[1] / 2.l;
}

inline void show_bode_diag(double* p_x, double* p_magnitude, double* p_phase_shift, uint64_t p_samples){
	static double* x = NULL, *magnitude = NULL, *phase_shift = NULL; 
	static uint64_t samples = 0;
	if(p_x){
		if(x)	free(x);
		x = p_x;
	} if(p_magnitude){
		if(magnitude)	free(magnitude);
		magnitude = p_magnitude;
	} if(p_phase_shift){
		if(phase_shift)	free(phase_shift);
		phase_shift = p_phase_shift;
	} if(p_samples){
		samples = p_samples;
	}

	if(x && magnitude && phase_shift && samples){
		ImGui::Begin("bode plot");
		if (ImPlot::BeginPlot("magnitue plot")){

			ImPlot::SetupAxis(ImAxis_X1, "frequency (rad/s)", ImPlotAxisFlags_None);
			ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
			ImPlot::SetupAxis(ImAxis_Y1, "magnitude (dB)", ImPlotAxisFlags_None);
			ImPlot::SetupAxis(ImAxis_Y2, "phase (deg)", ImPlotAxisFlags_Opposite);

			ImPlot::SetAxis(ImAxis_Y1);
			ImPlot::PlotLine("magnitude (dB)", x, magnitude, samples);
			ImPlot::SetAxis(ImAxis_Y2);
			ImPlot::PlotLine("phase (deg)", x, phase_shift, samples);

			ImPlot::EndPlot();
		}
		ImGui::End();
	}
}

inline void bode_plot(editor_t* editor, uint64_t nodeID1, uint64_t nodeID2, uint64_t samples, double start, double end){
	double* x = (double*) malloc(sizeof(double) * samples);
	double* magni = (double*) malloc(sizeof(double) * samples);
	double* phase = (double*) malloc(sizeof(double) * samples);
	start = log10(start); end = log10(end);
	double delta = (end - start)/samples;

	GiNaC::symbol s("s");
	GiNaC::matrix A, X, Z;

	constract_matrices(editor, &A, &X, &Z, s, numerical);

	GiNaC::ex result = pivodgos(A, Z);
	std::cout << result[nodeID1] << "\n" << result[nodeID2] << "\n";
	GiNaC::ex H = result[nodeID1]/result[nodeID2];

	for(uint64_t count = 0; count < samples; count++){
		double omega =  pow(10, (delta * count) + start);
		GiNaC::ex transfer_func = H.subs(s == omega*GiNaC::I);
		GiNaC::ex phi, mag;
		
		if(transfer_func != 0)
			mag = GiNaC::evalf(20 * (GiNaC::log(GiNaC::abs(transfer_func)) / GiNaC::log(10)));
		else{
			// sf llah ishl 3liha
			x[count] = omega;
			magni[count] = -INFINITY;
			phase[count] = 90;
			continue;
		}

		if(GiNaC::real_part(transfer_func) != 0)
			phi = GiNaC::evalf(GiNaC::atan(GiNaC::imag_part(transfer_func) / GiNaC::real_part(transfer_func)) * 180 / GiNaC::Pi);
		else
			// arctan(infinity) = pi/2
			phi = 90;
		x[count] = omega;
		// C++ moment
		magni[count] = GiNaC::ex_to<GiNaC::numeric>(mag).to_double();
		phase[count] = GiNaC::ex_to<GiNaC::numeric>(phi).to_double();
	}
	ImPlot::SetNextAxesToFit();
	show_bode_diag(x, magni, phase, samples);
}

inline void show_comp_menu(editor_t* editor){
	static ImGuiID popupID = 0;
	if(ImGui::BeginPopup("component menu")){
		if(ImGui::MenuItem("rotate by  90 deg"))
			for(auto comp : editor->selected_components)
				comp->quad.rot += 1.570755; // PI/2
		if(ImGui::MenuItem("rotate by -90 deg"))
			for(auto comp : editor->selected_components)
				comp->quad.rot -= 1.570755; // PI/2

		if(editor->selected_components.size() == 1){
			if(editor->selected_components[0]->definition.type == graph){
				if(ImGui::MenuItem("bode plot")){
					if((editor->selected_components[0]->pins[0].connected_node &&
					    editor->selected_components[0]->pins[1].connected_node )||
					   ((!editor->selected_components[0]->pins[0].connected_node->id) &&
					    (!editor->selected_components[0]->pins[1].connected_node->id))){
						popupID = ImHashStr("bode diagram options");
						ImGui::PushOverrideID(popupID);
						ImGui::OpenPopup("bode diagram options");
						ImGui::PopID();
					} else {
						popupID = ImHashStr("ERR both terminals should be connected");
						ImGui::PushOverrideID(popupID);
						ImGui::OpenPopup("ERR both terminals should be connected with non-ground nodes");
						ImGui::PopID();
					}
				}
			} else {
				if(ImGui::MenuItem("properies")){
					// a work around cuz I cant open a popup from a menu
					popupID = ImHashStr("comp properties");
					ImGui::PushOverrideID(popupID);
					ImGui::OpenPopup("comp properties");
					ImGui::PopID();
				}
			}
		}
		if(ImGui::MenuItem("remove component")){
			for(size_t i = 0; i < editor->selected_components.size(); i++){
				// free pins and remove linked links
				for(size_t j = 0; j < editor->links.size(); j++)
					for(size_t k = 0; k < editor->selected_components[i]->definition.num_pins; k++)
						if((editor->links[j].pins[0] == &editor->selected_components[i]->pins[k]) ||
						   (editor->links[j].pins[1] == &editor->selected_components[i]->pins[k]))
							editor->links.erase(editor->links.begin() + j);
				if(editor->selected_components[i]->definition.type == ground){
					for(size_t j = 0; j < editor->links.size(); j++){
						if((editor->links[j].pins[0]->connected_node == editor->selected_components[i]->pins[0].connected_node) ||
						   (editor->links[j].pins[1]->connected_node == editor->selected_components[i]->pins[0].connected_node) ){
							editor->links[j].pins[0]->connected_node = NULL;
							editor->links[j].pins[1]->connected_node = NULL;
							editor->links.erase(editor->links.begin() + j);
						}
					}
				}
				free(editor->selected_components[i]->pins);
				if(editor->selected_components[i]->L1)
					free(editor->selected_components[i]->L1);
				if(editor->selected_components[i]->L2)
					free(editor->selected_components[i]->L2);
				editor->components.erase(editor->components.begin() + (editor->selected_components[i] - (&editor->components[0])));
				// update IDs
				size_t count = 0;
				for(size_t j = 0; j < editor->components.size(); j++)
					if(editor->components[i].definition.type == editor->selected_components[i]->definition.type)
						editor->components[i].id = count++;
			}
		}

		ImGui::EndPopup();
	}

	if(popupID){
		ImGui::PushOverrideID(popupID);
		if(ImGui::BeginPopupModal("comp properties")){
			ImGui::Text("component settings");
			const char* value_name[] = {"", "groundness","resestance", "inductance", "capacitance", "k", "voltage",
			"current", "gain", "gain", "gain", "gain", ""};
			ImGui::InputDouble(value_name[editor->selected_components[0]->definition.type],
			                   &editor->selected_components[0]->caracteristic);
			if(editor->selected_components[0]->definition.type == curr_cont_curr_source ||
			   editor->selected_components[0]->definition.type == curr_cont_volt_source){
				static const char* currently_selected = NULL;
				if(ImGui::BeginCombo("contoller", currently_selected)){
					for(uint64_t i = 0; i < editor->components.size(); i++){
						if(editor->components[i].definition.type == indp_current_source){
							char name[32];
							snprintf(name, 31, "%s %ld", editor->components[i].definition.name, editor->components[i].id);
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
			} else if(editor->selected_components[0]->definition.type == coupled_inductors){
				ImGui::InputDouble("inductance 1", &editor->selected_components[0]->L1->caracteristic);
				ImGui::InputDouble("inductance 2", &editor->selected_components[0]->L2->caracteristic);
			}
			if(ImGui::Button("OK DOKIE"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		if(ImGui::BeginPopupModal("ERR both terminals should be connected")){
			ImGui::Text("behold, lay a thin eye upon both terminals, they shal not stay unreachable");
			if(ImGui::Button("OK DOKIE"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		if(ImGui::BeginPopupModal("bode diagram options")){
			static int samples = 1 << 15;
			static double start = 1e2;
			static double end = 1e6;
			ImGui::InputDouble("start", &start);
			ImGui::InputDouble("end", &end);
			ImGui::InputInt("samples", &samples);
			if(ImGui::Button("draw it !!")){
				bode_plot(editor, editor->selected_components[0]->pins[0].connected_node->id - 1,
				          editor->selected_components[0]->pins[1].connected_node->id - 1, samples, start, end);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}
}

inline void showeditormenu(editor_t* editor){
	if(ImGui::BeginPopup("editor menu")){
		ImVec2 mousepos = ImGui::GetMousePosOnOpeningCurrentPopup();
		if(ImGui::BeginMenu("add a component")){
			for(uint32_t i = 1; i < sizeof(components)/sizeof(components[0]); i++)
				if(ImGui::MenuItem(components[i].name))
					spawn_comp(editor,
					          (mousepos.x - editor->grid.offset[0]) / editor->grid.scale,
					          (mousepos.y - editor->grid.offset[1]) / editor->grid.scale,
					          components[i].type);
			ImGui::EndMenu();
		}
		if(ImGui::MenuItem("re-center the grid"))
			recenter_grid(editor);
		ImGui::EndPopup();
	}
}

inline void processInput(GLFWwindow* window, editor_t* editor){
	static double lastmousepos[2];
	static bool draged = false;
	static bool rightpressed = false;
	static bool leftpressed = false;
	static bool pin_was_selected = false;
	double minimum = 100 * editor->grid.scale;
	
	// Fix Input Bleeding: Only process GLFW input if the ImGui window is hovered
	if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
		return;

	double x, y;
	glfwGetCursorPos(window, &x, &y);

	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)){
		for(uint64_t i = 0; i < editor->components.size(); i++){
			bool mouseovernode = false;
			for(uint64_t j = 0; j < editor->components[i].definition.num_pins; j++){
				double diffx = x - editor->components[i].pins[j].pos[0];
				double diffy = y - editor->components[i].pins[j].pos[1];
				if((diffx*diffx + diffy*diffy) < minimum && !pin_was_selected){
					editor->components[i].pins[j].selected = true;
					editor->connector = &editor->components[i].pins[j];
					pin_was_selected = true;
					mouseovernode = true;
				}
			}
			if(mouse_over_quad(editor, x, y, &editor->components[i].quad) && (!mouseovernode && !editor->connector && !leftpressed)){
				// select component
				if(std::find(editor->selected_components.begin(), editor->selected_components.end(),
				             &editor->components[i]) == editor->selected_components.end())
					editor->selected_components = {&editor->components[i]};
				break;
			}
		}
		for(size_t i = 0; i < editor->selected_components.size(); i++)
			editor->selected_components[i]->quad.pos[0] += (x - lastmousepos[0]) / editor->grid.scale,
			editor->selected_components[i]->quad.pos[1] += (y - lastmousepos[1]) / editor->grid.scale;

	leftpressed = true;
	} if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE){
		if(!editor->selected_components.empty() && draged)
			editor->selected_components.clear();
		if(pin_was_selected){
			for(uint64_t i = 0; i < editor->components.size(); i++){
				for(uint64_t j = 0; j < editor->components[i].definition.num_pins; j++){
					double diffx = x - editor->components[i].pins[j].pos[0];
					double diffy = y - editor->components[i].pins[j].pos[1];
					if((diffx*diffx + diffy*diffy) < minimum){
						pin_t* joint = (pin_t*)malloc(sizeof(pin_t));
						joint->pos[0] = editor->connector->pos[0];
						joint->pos[1] = editor->components[i].pins[j].pos[1];
						joint->connected_node = editor->connector->connected_node;
						joint->selected = false;
						editor->pins.push_back(joint);
						spawn_link(editor, editor->connector, joint);
						spawn_link(editor, joint, &editor->components[i].pins[j]);
					}
				}
			}
			pin_was_selected = false;
			editor->connector->selected = false;
			editor->connector = NULL;
		}

		leftpressed = false;
	} if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)){
		if(lastmousepos[0] != x || lastmousepos[1] != y){
			editor->grid.offset[0] += x - lastmousepos[0];
			editor->grid.offset[1] += y - lastmousepos[1];
			for(size_t i = 0; i < editor->pins.size(); i++){
				editor->pins[i]->pos[0] += (x - lastmousepos[0]);
				editor->pins[i]->pos[1] += (y - lastmousepos[1]);
			}
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
	
	// keyboard input
	// do you want to spawn two things at the same frame ? nah uh
	static component_type wouldbespawn = undefined;
	if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		wouldbespawn = resistor;
	else if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		wouldbespawn = capacitor;
	else if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		wouldbespawn = inductor;
	else if(glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
		wouldbespawn = indp_voltage_source;
	else if(glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		wouldbespawn = indp_current_source;
	else if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		wouldbespawn = ground;
	else if(glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		wouldbespawn = coupled_inductors;
	else if(glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		wouldbespawn = operational_amplifier;
	else{
		// if none of the above is pressed, spawn the last presed comp
		if(wouldbespawn != undefined){
			spawn_comp(editor, (x - editor->grid.offset[0]) / editor->grid.scale,
			           (y - editor->grid.offset[1]) / editor->grid.scale, wouldbespawn);
			wouldbespawn = undefined;
		}
	}
};

inline void drawgrid(ImDrawList* drawlist, editor_t* editor){
	const float lgrid = editor->grid.step * editor->grid.scale;
	int markX = editor->grid.offset[0] / lgrid,
	    markY = editor->grid.offset[1] / lgrid;

#ifdef LIGHTTHEME
	for(float x = fmodf(editor->grid.offset[0], lgrid); x < editor->resol[0]; x += lgrid, markX--)
		drawlist->AddLine(ImVec2(x, 0.f), ImVec2(x, editor->resol[1]),
		                  markX % 8 ? IM_COL32(200, 200, 200, 150) : IM_COL32(220, 220, 220, 100));
	for(float y = fmodf(editor->grid.offset[1], lgrid); y < editor->resol[1]; y += lgrid, markY--)
		drawlist->AddLine(ImVec2(0.f, y), ImVec2(editor->resol[0], y),
		                  markY % 8 ? IM_COL32(200, 200, 200, 150) : IM_COL32(220, 220, 220, 100));
#else
	for(float x = fmodf(editor->grid.offset[0], lgrid); x < editor->resol[0]; x += lgrid, markX--)
		drawlist->AddLine(ImVec2(x, 0.f), ImVec2(x, editor->resol[1]),
		                  markX % 8 ? IM_COL32(60, 60, 70, 150) : IM_COL32(45, 45, 50, 100));
	for(float y = fmodf(editor->grid.offset[1], lgrid); y < editor->resol[1]; y += lgrid, markY--)
		drawlist->AddLine(ImVec2(0.f, y), ImVec2(editor->resol[0], y),
		                  markY % 8 ? IM_COL32(60, 60, 70, 150) : IM_COL32(45, 45, 50, 100));
#endif

	// draw origin crosshair
	drawlist->AddLine(ImVec2(editor->grid.offset[0] - 7, editor->grid.offset[1] - 7),
	                  ImVec2(editor->grid.offset[0] + 7, editor->grid.offset[1] + 7), IM_COL32(200, 200, 200, 200), 2.0f);
	drawlist->AddLine(ImVec2(editor->grid.offset[0] + 7, editor->grid.offset[1] - 7),
	                  ImVec2(editor->grid.offset[0] - 7, editor->grid.offset[1] + 7), IM_COL32(200, 200, 200, 200), 2.0f);

}

inline void draw_comps(ImDrawList* drawlist, editor_t* editor){
	for(uint32_t i = 0; i != editor->components.size(); i++){
		const Quad_t* q = &editor->components[i].quad;
		const double cosr = cos(q->rot), sinr = sin(q->rot);

		ImVec2 p[4];
		p[0] = ImVec2(editor->grid.offset[0] + ((q->pos[0] - (((q->dims[0] * cosr) + (q->dims[1] * sinr)) / 4.l)) * editor->grid.scale),
		              editor->grid.offset[1] + ((q->pos[1] + (((q->dims[1] * cosr) - (q->dims[0] * sinr)) / 4.l)) * editor->grid.scale));
		p[1] = ImVec2(editor->grid.offset[0] + ((q->pos[0] + (((q->dims[0] * cosr) - (q->dims[1] * sinr)) / 4.l)) * editor->grid.scale),
		              editor->grid.offset[1] + ((q->pos[1] + (((q->dims[1] * cosr) + (q->dims[0] * sinr)) / 4.l)) * editor->grid.scale));
		p[2] = ImVec2(editor->grid.offset[0] + ((q->pos[0] + (((q->dims[0] * cosr) + (q->dims[1] * sinr)) / 4.l)) * editor->grid.scale),
		              editor->grid.offset[1] + ((q->pos[1] - (((q->dims[1] * cosr) - (q->dims[0] * sinr)) / 4.l)) * editor->grid.scale));
		p[3] = ImVec2(editor->grid.offset[0] + ((q->pos[0] - (((q->dims[0] * cosr) - (q->dims[1] * sinr)) / 4.l)) * editor->grid.scale),
		              editor->grid.offset[1] + ((q->pos[1] - (((q->dims[1] * cosr) + (q->dims[0] * sinr)) / 4.l)) * editor->grid.scale));

		drawlist->AddImageQuad(q->texID, p[0], p[1], p[2], p[3]);

		ImVec2 npos;
		for(uint64_t j = 0; j < editor->components[i].definition.num_pins; j++){
			uint8_t indices[] = {0, 1, 3, 2};
			uint8_t a = indices[j%2], b = indices[(j%2) + 2];
			if(editor->components[i].definition.num_pins == 1)
				npos = ImVec2((p[2].x + p[3].x) / 2, (p[2].y + p[3].y) / 2);
			else if(editor->components[i].definition.num_pins == 2)
				npos = ImVec2((p[a].x + p[b].x) / 2, (p[a].y + p[b].y) / 2);
			else if(editor->components[i].definition.num_pins == 3)
				if(j == 1)
					npos = ImVec2((p[a].x + p[b].x) / 2,(p[a].y + p[b].y) / 2);
				else
					npos = ImVec2((p[a].x + p[b].x) / 2, (((j >> 1) + 1) * (p[a].y - p[b].y) / 3) + p[b].y);
			else if(editor->components[i].definition.num_pins == 4)
					npos = ImVec2((((j >> 1) + 1) * (p[a].x - p[b].x) / 3) + p[b].x,
				                  (((j >> 1) + 1) * (p[a].y - p[b].y) / 3) + p[b].y);
			editor->components[i].pins[j].pos[0] = npos.x, editor->components[i].pins[j].pos[1] = npos.y;
#ifdef LIGHTTHEME
			ImColor color = IM_COL32(0, 0, 0, 255);
#else
			ImColor color = IM_COL32(220, 220, 220, 255);
#endif
			if(editor->components[i].pins[j].selected){
				color = IM_COL32(0, 255, 0, 255);
			}
			drawlist->AddCircleFilled(npos, 7.5 * editor->grid.scale, color);
		}
	}
}

inline void drawlinks(ImDrawList* drawlist, editor_t* editor){

	for(size_t i = 0; i < editor->links.size(); i++){
		drawlist->AddLine(ImVec2(editor->links[i].pins[0]->pos[0], editor->links[i].pins[0]->pos[1]),
		                  ImVec2(editor->links[i].pins[1]->pos[0], editor->links[i].pins[1]->pos[1]),
#ifdef LIGHTTHEME
		                  IM_COL32(0, 0, 0, 255));
#else
		                  IM_COL32(200, 200, 255, 255));
#endif
	}

	if(editor->connector){
		ImVec2 mousecord = ImGui::GetMousePos();
		ImVec2 tmp = abs(mousecord.x - editor->connector->pos[0]) > abs(mousecord.y - editor->connector->pos[1]) ?
		             ImVec2(mousecord.x, editor->connector->pos[1]) : ImVec2(editor->connector->pos[0], mousecord.y);
		drawlist->AddLine(ImVec2(editor->connector->pos[0], editor->connector->pos[1]), tmp, IM_COL32(255, 0, 0, 255));
		drawlist->AddLine(tmp, ImGui::GetMousePos(), IM_COL32(255, 0, 0, 255));
	}
}

inline void Menu(){
	static ImGuiID aboutpopupID = 0;
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
		//if(ImGui::BeginMenu("solve circuit")){
		//	if(ImGui::MenuItem("solve for G matrix")){}
		//	ImGui::EndMenu();
		//}

		if (ImGui::BeginMenu("Help")){
			if (ImGui::MenuItem("About", nullptr, false, true)){
				aboutpopupID = ImHashStr("about window");
				ImGui::PushOverrideID(aboutpopupID);
				ImGui::OpenPopup( "about window" );
				ImGui::PopID();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	// work around not being able to init popup wondows from menu items
	if(aboutpopupID){
		ImGui::PushOverrideID(aboutpopupID);
		if(ImGui::BeginPopupModal("about window")){
			ImGui::Text("dont u know me");
			ImGui::Text("shame on you");
			if(ImGui::Button("OK"))	ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}
}

inline void Dockspace(GLFWwindow* window, editor_t* editor){
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
	for(auto pin : editor->pins)
		ImGui::Text("%f, %f, %p", pin->pos[0], pin->pos[1], pin->connected_node);
	for(auto link : editor->links)
		ImGui::Text("%p, %p, %p", link.pins[0], link.pins[1], &link);
	ImGui::End();

	ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_None);
	ImGui::End();

	ImGui::Begin("Explorer", nullptr, ImGuiWindowFlags_None);
	ImGui::Text("numnoded : %lu", get_num_nodes(editor));
	for(size_t i = 0; i < editor->components.size(); i++){
		ImGui::Text("%s %lu, pins:", editor->components[i].definition.name, editor->components[i].id);
		ImGui::SameLine();
		for(uint8_t j = 0; j < editor->components[i].definition.num_pins; j++){
			ImGui::Text("%lu", editor->components[i].pins[j].connected_node ? editor->components[i].pins[j].connected_node->id : 420);
			ImGui::SameLine();
		}
		ImGui::NewLine();
	}
	ImGui::End();

	ImGui::Begin("Diagram", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	processInput(window, editor);
	ImDrawList* drawlist = ImGui::GetWindowDrawList();
	drawgrid(drawlist, editor);
	draw_comps(drawlist, editor);
	drawlinks(drawlist, editor);
	ImGui::End();
}

#endif //GUI_H_
