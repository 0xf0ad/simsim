#ifndef GUI_H_
#define GUI_H_

#include "components.h"
#include "fdtd_solver.h"
#include "matrix.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "implot/implot.h"
#include "ed.h"
#include <GLFW/glfw3.h>

// Application mode — defined here, instantiated in main.cpp
enum app_mode_t { MODE_SELECT, MODE_CIRCUIT, MODE_EM };
extern app_mode_t app_mode;

inline void show_mode_select(app_mode_t* mode) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(520, 190), ImGuiCond_Always);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar   |
        ImGuiWindowFlags_NoResize     |
        ImGuiWindowFlags_NoMove       |
        ImGuiWindowFlags_NoScrollbar  |
        ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("##mode_select", NULL, flags);

    // Title
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Select Simulation Mode").x) * 0.5f);
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Select Simulation Mode");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    float btn_w = 200.0f, btn_h = 110.0f;
    float total_w = btn_w * 2 + ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - total_w) * 0.5f + ImGui::GetStyle().WindowPadding.x);

    // --- Circuit button ---
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.15f, 0.35f, 0.55f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.50f, 0.75f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.10f, 0.25f, 0.45f, 1.0f));
    if (ImGui::Button("##circuit_btn", ImVec2(btn_w, btn_h)))
        *mode = MODE_CIRCUIT;
    ImGui::PopStyleColor(3);

    // overlay text on the button
    ImVec2 btn_min = ImGui::GetItemRectMin();
    ImVec2 btn_max = ImGui::GetItemRectMax();
    ImVec2 btn_ctr((btn_min.x + btn_max.x) * 0.5f, (btn_min.y + btn_max.y) * 0.5f);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const char* lbl1 = "Circuit";
    const char* lbl2 = "AC Analysis";
    ImVec2 s1 = ImGui::CalcTextSize(lbl1);
    ImVec2 s2 = ImGui::CalcTextSize(lbl2);
    dl->AddText(ImVec2(btn_ctr.x - s1.x*0.5f, btn_ctr.y - s1.y - 4), IM_COL32(255,255,255,255), lbl1);
    dl->AddText(ImVec2(btn_ctr.x - s2.x*0.5f, btn_ctr.y + 4),        IM_COL32(200,230,255,220), lbl2);

	ImGui::SameLine();

    // --- EM button ---
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.15f, 0.45f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.65f, 0.42f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.10f, 0.32f, 0.22f, 1.0f));
    if (ImGui::Button("##em_btn", ImVec2(btn_w, btn_h)))
        *mode = MODE_EM;
    ImGui::PopStyleColor(3);

    ImVec2 b2_min = ImGui::GetItemRectMin();
    ImVec2 b2_max = ImGui::GetItemRectMax();
    ImVec2 b2_ctr((b2_min.x + b2_max.x) * 0.5f, (b2_min.y + b2_max.y) * 0.5f);
    const char* lbl3 = "EM Waves";
    const char* lbl4 = "Medium Editor";
    ImVec2 s3 = ImGui::CalcTextSize(lbl3);
    ImVec2 s4 = ImGui::CalcTextSize(lbl4);
    dl->AddText(ImVec2(b2_ctr.x - s3.x*0.5f, b2_ctr.y - s3.y - 4), IM_COL32(255,255,255,255), lbl3);
    dl->AddText(ImVec2(b2_ctr.x - s4.x*0.5f, b2_ctr.y + 4),        IM_COL32(200,255,220,220), lbl4);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::SetCursorPosX(ImGui::GetStyle().WindowPadding.x);
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Choose a mode to begin. You can switch modes from the View menu.").x) * 0.5f);
    ImGui::TextDisabled("Choose a mode to begin. You can switch modes from the View menu.");

    ImGui::End();
}

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
			for(size_t i = 0; i < editor->selected_components.size(); i++)
				editor->selected_components[i]->quad.rot += 1.570755; // pi/2
		if(ImGui::MenuItem("rotate by -90 deg"))
			for(size_t i = 0; i < editor->selected_components.size(); i++)
				editor->selected_components[i]->quad.rot -= 1.570755; // pi/2

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

inline bool point_near_pin(double x, double y, pin_t* pin, double threshold){
	double dx = x - pin->pos[0];
	double dy = y - pin->pos[1];
	return (dx*dx + dy*dy) < threshold;
}

inline pin_t* find_pin_at(editor_t* editor, double x, double y, double threshold){
	for(size_t i = 0; i < editor->components.size(); i++)
		for(uint8_t j = 0; j < editor->components[i].definition.num_pins; j++)
			if(point_near_pin(x, y, &editor->components[i].pins[j], threshold))
				return &editor->components[i].pins[j];
	return NULL;
}

inline component_t* find_component_at(editor_t* editor, double x, double y){
	for(size_t i = 0; i < editor->components.size(); i++)
		if(mouse_over_quad(editor, x, y, &editor->components[i].quad))
			return &editor->components[i];
	return NULL;
}

inline bool pin_belongs_to(pin_t* pin, component_t* comp){
	for(uint8_t i = 0; i < comp->definition.num_pins; i++)
		if(&comp->pins[i] == pin)
			return true;
	return false;
}

inline void update_joints_for_components(editor_t* editor, std::vector<component_t*>& moved_comps){
	for(size_t i = 0; i < editor->links.size(); i++){
		link_t* link = &editor->links[i];
		pin_t* joint = NULL;
		pin_t* comp_pin = NULL;

		for(size_t j = 0; j < editor->pins.size(); j++){
			if(link->pins[0] == editor->pins[j]){
				joint = editor->pins[j];
				comp_pin = link->pins[1];
				break;
			} else if(link->pins[1] == editor->pins[j]){
				joint = editor->pins[j];
				comp_pin = link->pins[0];
				break;
			}
		}

		if(!joint || !comp_pin) continue;

		bool pin_moved = false;
		for(size_t j = 0; j < moved_comps.size(); j++){
			if(pin_belongs_to(comp_pin, moved_comps[j])){
				pin_moved = true;
				break;
			}
		}

		if(pin_moved){
			for(size_t k = 0; k < editor->links.size(); k++){
				if(k == i) continue;
				link_t* other = &editor->links[k];
				pin_t* other_pin = NULL;
				if(other->pins[0] == joint)
					other_pin = other->pins[1];
				else if(other->pins[1] == joint)
					other_pin = other->pins[0];

				if(other_pin){
					joint->pos[0] = comp_pin->pos[0];
					joint->pos[1] = other_pin->pos[1];
					break;
				}
			}
		}
	}
}


inline void processInput(GLFWwindow* window, editor_t* editor){
	static double lastmousepos[2];
	static bool grid_dragged = false;
	static bool rightpressed = false;
	static bool leftpressed = false;
	static bool pin_was_selected = false;
	static bool dragging_component = false;
	const double pin_threshold = 100 * editor->grid.scale;

	if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
		return;

	double x, y;
	glfwGetCursorPos(window, &x, &y);

	double dx = x - lastmousepos[0];
	double dy = y - lastmousepos[1];
	bool mouse_moved = (dx != 0 || dy != 0);

	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
		if(!leftpressed){
			if(!dragging_component){
				pin_t* clicked_pin = find_pin_at(editor, x, y, pin_threshold);
				if(clicked_pin){
					clicked_pin->selected = true;
					editor->connector = clicked_pin;
					pin_was_selected = true;
				}
			}
			
			if(!pin_was_selected && !editor->connector){
				component_t* clicked_comp = find_component_at(editor, x, y);
				if(clicked_comp){
					editor->selected_components = {clicked_comp};
					dragging_component = true;
				}
			}
		}
		
		if(dragging_component && mouse_moved){
			for(size_t i = 0; i < editor->selected_components.size(); i++){
				editor->selected_components[i]->quad.pos[0] += dx / editor->grid.scale;
				editor->selected_components[i]->quad.pos[1] += dy / editor->grid.scale;
			}
			update_joints_for_components(editor, editor->selected_components);
		}

		leftpressed = true;
	}
	else if(leftpressed){
		leftpressed = false;

		if(dragging_component){
			editor->selected_components.clear();
			dragging_component = false;
		}

		if(pin_was_selected && editor->connector){
			pin_t* target_pin = find_pin_at(editor, x, y, pin_threshold);
			if(target_pin && target_pin != editor->connector){
				pin_t* joint = (pin_t*)malloc(sizeof(pin_t));
				joint->pos[0] = editor->connector->pos[0];
				joint->pos[1] = target_pin->pos[1];
				joint->connected_node = editor->connector->connected_node;
				joint->selected = false;
				editor->pins.push_back(joint);
				spawn_link(editor, editor->connector, joint);
				spawn_link(editor, joint, target_pin);
			}
			editor->connector->selected = false;
			editor->connector = NULL;
			pin_was_selected = false;
		}
	}
	
	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS){
		if(mouse_moved){
			editor->grid.offset[0] += dx;
			editor->grid.offset[1] += dy;
			for(size_t i = 0; i < editor->pins.size(); i++){
				editor->pins[i]->pos[0] += dx;
				editor->pins[i]->pos[1] += dy;
			}
			grid_dragged = true;
		}
		rightpressed = true;
	}
	else if(rightpressed){
		if(!grid_dragged){
			component_t* clicked_comp = find_component_at(editor, x, y);
			if(clicked_comp){
				ImGui::OpenPopup("component menu");
				editor->selected_components = {clicked_comp};
			} else {
				ImGui::OpenPopup("editor menu");
			}
		}
		grid_dragged = false;
		rightpressed = false;
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

		// DONT TOUCH
		// this math was a nightmare to figure out
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
			if(editor->components[i].pins[j].selected)
				color = IM_COL32(0, 255, 0, 255);
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
			if (ImGui::MenuItem("New", NULL, false, true)){}
			if (ImGui::MenuItem("Open", NULL, false, true)){}
			ImGui::Separator();
			if (ImGui::MenuItem("Save", NULL, false, true)){}
			if (ImGui::MenuItem("Save As...", NULL, false, true)){}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", NULL, false, true)){}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")){
			if (ImGui::MenuItem("Circuit AC Analysis"))
				app_mode = MODE_CIRCUIT;
			if (ImGui::MenuItem("EM Wave Simulation"))
				app_mode = MODE_EM;
			ImGui::Separator();
			if (ImGui::MenuItem("Mode Selection Screen"))
				app_mode = MODE_SELECT;
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help")){
			if (ImGui::MenuItem("About", NULL, false, true)){
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


inline void em_editor_panel(GLFWwindow* window, editor_t* c, ImGuiID dsid) {
	// ---- Toolbar / sidebar ------------------------------------------------
	ImGui::SetNextWindowDockID(dsid, ImGuiCond_FirstUseEver);
	ImGui::Begin("EM Material Palette", NULL, ImGuiWindowFlags_None);

	ImGui::SeparatorText("Draw Mode");
	if (ImGui::RadioButton("Rectangle",  c->draw_mode == EM_SHAPE_RECT))
		c->draw_mode = EM_SHAPE_RECT;
	ImGui::SameLine();
	if (ImGui::RadioButton("Circle",     c->draw_mode == EM_SHAPE_CIRCLE))
		c->draw_mode = EM_SHAPE_CIRCLE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Polygon",    c->draw_mode == EM_SHAPE_POLYGON))
		c->draw_mode = EM_SHAPE_POLYGON;

	ImGui::Spacing();
	ImGui::SeparatorText("Active Material");

	ImGui::InputText("Name", c->active_material.name, 32);
	ImGui::InputFloat("Conductivity (S/m)",  &c->active_material.conductivity, 0, 0, "%.3e");
	ImGui::InputFloat("Permittivity (eps_r)", &c->active_material.permittivity, 0.1f, 1.0f, "%.2f");
	ImGui::InputFloat("Permeability (mu_r)",  &c->active_material.permeability, 0.1f, 1.0f, "%.2f");
	ImGui::ColorEdit4("Fill color", (float*)&c->active_material.color);

	ImGui::Spacing();
	ImGui::SeparatorText("Presets");
	for (long unsigned int i = 0; i < sizeof(em_presets)/sizeof(em_presets[0]); i++) {
		ImGui::PushID(i);
		ImVec4 pc = em_presets[i].color;
		pc.w = 1.0f;
		ImGui::ColorButton("##col", pc, ImGuiColorEditFlags_NoTooltip, ImVec2(14,14));
		ImGui::SameLine();
		if (ImGui::Button(em_presets[i].name))
			c->active_material = em_presets[i];
		ImGui::PopID();
	}

	ImGui::Spacing();
	ImGui::SeparatorText("Selected Shape");
	if (c->selected_idx >= 0 && c->selected_idx < (int)c->shapes.size()) {
		em_shape_t* sel = &c->shapes[c->selected_idx];
		ImGui::InputText("Name##sel",  sel->material.name, 32);
		ImGui::InputFloat("Conductivity##sel",  &sel->material.conductivity, 0, 0, "%.3e");
		ImGui::InputFloat("Permittivity##sel",  &sel->material.permittivity, 0.1f, 1.0f, "%.2f");
		ImGui::InputFloat("Permeability##sel",  &sel->material.permeability, 0.1f, 1.0f, "%.2f");
		ImGui::ColorEdit4("Color##sel", (float*)&sel->material.color);
		if (ImGui::Button("Delete shape")) {
			c->shapes.erase(c->shapes.begin() + c->selected_idx);
			c->selected_idx = -1;
		}
	} else {
		ImGui::TextDisabled("(click a shape to select)");
	}

	ImGui::Spacing();
	ImGui::SeparatorText("Canvas");
	if (ImGui::Button("Clear all")) {
		c->shapes.clear();
		c->selected_idx = -1;
	}
	ImGui::SameLine();
	if (ImGui::Button("Re-center")) 
		c->grid.offset[0] = c->grid.offset[1] = 0;

	ImGui::Text("Shapes: %d", (int)c->shapes.size());
	
	ImGui::Spacing();
	ImGui::SeparatorText("Simulation");

	if (!c->fdtd.initialized) {
		if (ImGui::Button("Initialize Grid")) {
			fdtd_init_grid(c, 0.001f);  // 1mm cells
		}
		ImGui::TextWrapped("Grid not initialized. Click to create simulation grid from shapes.");
	} else {
		ImGui::Text("Grid: %dx%d cells", c->fdtd.nx, c->fdtd.ny);
		ImGui::Text("Cell size: %.3f mm", c->fdtd.dx * 1000.0f);
		ImGui::Text("Time step: %d", c->fdtd.time_step);
		ImGui::Text("Max field: %.2e V/m", c->fdtd.max_field);

		if (c->fdtd.running) {
			if (ImGui::Button("Pause")) c->fdtd.running = false;
			ImGui::SameLine();
			if (ImGui::Button("Reset")) {
				c->fdtd.time_step = 0;
				std::fill(c->fdtd.Ez.begin(), c->fdtd.Ez.end(), 0.0f);
				std::fill(c->fdtd.Hx.begin(), c->fdtd.Hx.end(), 0.0f);
				std::fill(c->fdtd.Hy.begin(), c->fdtd.Hy.end(), 0.0f);
			}
		} else {
			if (ImGui::Button("Run")) c->fdtd.running = true;
			ImGui::SameLine();
			if (ImGui::Button("Step")) fdtd_step(c);
		}

		if (ImGui::Button("Reinitialize")) {
			fdtd_init_grid(c, 0.001f);
		}
	}

	ImGui::Spacing();
	ImGui::SeparatorText("Sources");

	if (ImGui::Button("Add Point Source")) {
		em_source_t src = {
			.type = EM_SOURCE_POINT,
			.pos = ImVec2(5.0f, 5.0f),
			.frequency = 1e9f,  // 1 GHz
			.amplitude = 1.0f,
			.phase = 0.0f,
			.angle = 0.0f,
			.active = true,
		};
		snprintf(src.name, 32, "Source%d", (int)c->sources.size());
		c->sources.push_back(src);
	}
	ImGui::SameLine();
	if (ImGui::Button("Add Plane Wave")) {
		em_source_t src = {
			.type = EM_SOURCE_PLANE,
			.pos = ImVec2(2.0f, 5.0f),
			.frequency = 1e9f,
			.amplitude = 1.0f,
			.phase = 0.0f,
			.angle = 0.0f,
			.active = true,
		};
		snprintf(src.name, 32, "Plane%d", (int)c->sources.size());
		c->sources.push_back(src);
	}

	for (int i = 0; i < (int)c->sources.size(); i++) {
		ImGui::PushID(i);
		bool sel = (c->selected_source_idx == i);
		if (ImGui::Selectable(c->sources[i].name, sel)) {
			c->selected_source_idx = i;
			c->selected_idx = -1;
			c->selected_probe_idx = -1;
		}
		ImGui::PopID();
	}

	if (c->selected_source_idx >= 0 && c->selected_source_idx < (int)c->sources.size()) {
		em_source_t* src = &c->sources[c->selected_source_idx];
		ImGui::Spacing();
		ImGui::Text("Source Properties:");
		ImGui::Checkbox("Active##src", &src->active);
		ImGui::InputFloat("Frequency (Hz)##src", &src->frequency, 0, 0, "%.3e");
		ImGui::InputFloat("Amplitude##src", &src->amplitude, 0.1f, 1.0f, "%.2f");
		ImGui::InputFloat("Phase (rad)##src", &src->phase, 0.1f, 1.0f, "%.2f");
		if (src->type == EM_SOURCE_PLANE) {
			ImGui::SliderAngle("Angle##src", &src->angle);
		}
		if (ImGui::Button("Delete Source")) {
			c->sources.erase(c->sources.begin() + c->selected_source_idx);
			c->selected_source_idx = -1;
		}
	}

	ImGui::Spacing();
	ImGui::SeparatorText("Probes");

	if (ImGui::Button("Add E-Field Probe")) {
		em_probe_t probe = {
			.type = EM_PROBE_E_FIELD,
			.pos = ImVec2(7.0f, 5.0f),
			.value = 0.0f,
			.active = true,
		};
		snprintf(probe.name, 32, "E%d", (int)c->probes.size());
		c->probes.push_back(probe);
	}
	ImGui::SameLine();
	if (ImGui::Button("Add H-Field Probe")) {
		em_probe_t probe = {
			.type = EM_PROBE_H_FIELD,
			.pos = ImVec2(7.0f, 5.0f),
			.value = 0.0f,
			.active = true,
		};
		snprintf(probe.name, 32, "H%d", (int)c->probes.size());
		c->probes.push_back(probe);
	}

	for (int i = 0; i < (int)c->probes.size(); i++) {
		ImGui::PushID(100 + i);
		bool sel = (c->selected_probe_idx == i);
		if (ImGui::Selectable(c->probes[i].name, sel)) {
			c->selected_probe_idx = i;
			c->selected_idx = -1;
			c->selected_source_idx = -1;
		}
		ImGui::PopID();
	}

	if (c->selected_probe_idx >= 0 && c->selected_probe_idx < (int)c->probes.size()) {
		em_probe_t* probe = &c->probes[c->selected_probe_idx];
		ImGui::Spacing();
		ImGui::Text("Probe Properties:");
		ImGui::Checkbox("Active##probe", &probe->active);
		ImGui::Text("Value: %.3e", probe->value);
		if (ImGui::Button("Delete Probe")) {
			c->probes.erase(c->probes.begin() + c->selected_probe_idx);
			c->selected_probe_idx = -1;
		}
	}

	ImGui::Spacing();
	ImGui::SeparatorText("Help");
	ImGui::TextWrapped(
		"Left-click drag: draw shape\n"
		"Polygon: click vertices, double-click to close\n"
		"Right-drag: pan\n"
		"Scroll: zoom\n"
		"Click shape: select\n"
		"Esc: cancel polygon"
	);

	ImGui::End();

	// ---- Canvas -----------------------------------------------------------
	// Dock into the EM dockspace on first run
	ImGui::SetNextWindowDockID(dsid, ImGuiCond_FirstUseEver);
	ImGui::Begin("EM Canvas", NULL,
					ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImVec2 huh = ImGui::GetCursorScreenPos();
	ImVec2 huh2 = ImGui::GetContentRegionAvail();
	ImVec2 canvas_size   = ImVec2(huh.x + huh2.x, huh.y + huh2.y);

	// invisible button to capture input
	ImGui::InvisibleButton("em_canvas_area", canvas_size,
							ImGuiButtonFlags_MouseButtonLeft  |
							ImGuiButtonFlags_MouseButtonRight |
							ImGuiButtonFlags_MouseButtonMiddle);
	bool hovered = ImGui::IsItemHovered();
	bool lclick  = ImGui::IsItemClicked(ImGuiMouseButton_Left);
	bool rdown   = ImGui::IsMouseDown(ImGuiMouseButton_Right);

	ImVec2 mousepos = ImGui::GetMousePos();
	ImVec2 mouse_canvas = ImVec2(
		(mousepos.x - c->grid.offset[0]) / c->grid.scale,
		(mousepos.y - c->grid.offset[1]) / c->grid.scale);
	// ---- Right-drag to pan ------------------------------------------------
	if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		c->pan_last = mousepos;
	if (rdown && hovered) {
		ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right, 0.0f);
		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
		c->grid.offset[0] += delta.x;
		c->grid.offset[1] += delta.y;
	}

	// ---- Left-click: draw / select / edit ----------------------------------------
	if (hovered && lclick) {
		// Check if clicking on edit handle first
		if (c->selected_idx >= 0 && c->selected_idx < (int)c->shapes.size()) {
			double threshold = 10.l;
			const em_shape_t* s = &c->shapes[c->selected_idx];

			if (s->type == EM_SHAPE_RECT){
				double handles[8][2] = {
					{s->p0.x                   , s->p0.y                  },
					{(s->p0.x + s->p1.x) * 0.5f, s->p0.y                  },
					{s->p1.x                   , s->p0.y                  },
					{s->p1.x                   ,(s->p0.y + s->p1.y) * 0.5f},
					{s->p1.x                   , s->p1.y                  },
					{(s->p0.x + s->p1.x) * 0.5f, s->p1.y                  },
					{s->p0.x                   , s->p1.y                  },
					{s->p0.x                   ,(s->p0.y + s->p1.y) * 0.5f},
				};

				for (int i = 0; i < 8; i++){
					double dx = mouse_canvas.x - handles[i][0];
					double dy = mouse_canvas.y - handles[i][1];
					if (dx*dx + dy*dy < threshold){
						c->editing_shape = true;
						c->edit_handle = i;
						break;
					}
				}
			} else if (s->type == EM_SHAPE_CIRCLE){
				double r = s->p1.x;
				double handles[4][2] = {
					{s->p0.x + r, s->p0.y    },
					{s->p0.x    , s->p0.y + r},
					{s->p0.x - r, s->p0.y    },
					{s->p0.x    , s->p0.y - r},
				};

				for (int i = 0; i < 4; i++){
					double dx = mouse_canvas.x - handles[i][0];
					double dy = mouse_canvas.y - handles[i][1];
					if (dx*dx + dy*dy < threshold){
						c->editing_shape = true;
						c->edit_handle = i;
						break;
					}
				}
			}
		}

		if (!c->editing_shape) {
			if (c->draw_mode == EM_SHAPE_POLYGON) {
				// double-click closes polygon
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					if (c->poly_wip.size() >= 3) {
						em_shape_t s;
						s.type     = EM_SHAPE_POLYGON;
						s.material = c->active_material;
						s.poly_pts = c->poly_wip;
						s.selected = false;
						c->shapes.push_back(s);
					}
					c->poly_wip.clear();
					c->drawing = false;
				} else {
					c->poly_wip.push_back(mouse_canvas);
					c->drawing = true;
				}
			} else {
				if (!c->drawing) {
					// Check sources
					bool hit_src = false;
					for (int i = (int)c->sources.size() - 1; i >= 0; i--) {
						float dx = mouse_canvas.x - c->sources[i].pos.x;
						float dy = mouse_canvas.y - c->sources[i].pos.y;
						if (dx*dx + dy*dy < 0.3f*0.3f) {
							c->selected_source_idx = i;
							c->selected_idx = -1;
							c->selected_probe_idx = -1;
							hit_src = true;
							break;
						}
					}

					// Check probes
					if (!hit_src) {
						for (int i = (int)c->probes.size() - 1; i >= 0; i--) {
							float dx = mouse_canvas.x - c->probes[i].pos.x;
							float dy = mouse_canvas.y - c->probes[i].pos.y;
							if (dx*dx + dy*dy < 0.3f*0.3f) {
								c->selected_probe_idx = i;
								c->selected_idx = -1;
								c->selected_source_idx = -1;
								hit_src = true;
								break;
							}
						}
					}

					// Check shapes
					if (!hit_src) {
						bool hit = false;
						for (int i = (int)c->shapes.size() - 1; i >= 0; i--) {
							bool doit = false;
							switch (c->shapes[i].type) {
								case EM_SHAPE_RECT: {
									double x0 = c->shapes[i].p0.x < c->shapes[i].p1.x ? c->shapes[i].p0.x : c->shapes[i].p1.x;
									double x1 = c->shapes[i].p0.x > c->shapes[i].p1.x ? c->shapes[i].p0.x : c->shapes[i].p1.x;
									double y0 = c->shapes[i].p0.y < c->shapes[i].p1.y ? c->shapes[i].p0.y : c->shapes[i].p1.y;
									double y1 = c->shapes[i].p0.y > c->shapes[i].p1.y ? c->shapes[i].p0.y : c->shapes[i].p1.y;
									doit = mouse_canvas.x >= x0 && mouse_canvas.x <= x1 && mouse_canvas.y >= y0 && mouse_canvas.y <= y1;
									break;
								}
								case EM_SHAPE_CIRCLE: {
									float dx = mouse_canvas.x - c->shapes[i].p0.x, dy = mouse_canvas.y - c->shapes[i].p0.y;
									doit = (dx*dx + dy*dy) <= c->shapes[i].p0.x * c->shapes[i].p0.x;
									break;
								}
								case EM_SHAPE_POLYGON: {
									bool inside = false;
									size_t size = c->shapes[i].poly_pts.size();
									for (size_t i = 0, j = size - 1; i < size; j = i++) {
										float xi = c->shapes[i].poly_pts[i].x, yi = c->shapes[i].poly_pts[i].y;
										float xj = c->shapes[i].poly_pts[j].x, yj = c->shapes[i].poly_pts[j].y;
										if (((yi > mouse_canvas.y) != (yj > mouse_canvas.y)) &&
											(mouse_canvas.x < (xj - xi) * (mouse_canvas.y - yi) / (yj - yi) + xi))
											doit = !doit;
									}
									break;
								}
							}
							if (doit){
								c->selected_idx = i;
								c->selected_source_idx = -1;
								c->selected_probe_idx = -1;
								hit = true;
								break;
							}
						}
						if (!hit) {
							c->selected_idx = -1;
							c->selected_source_idx = -1;
							c->selected_probe_idx = -1;
							c->draw_start = mouse_canvas;
							c->drawing    = true;
						}
					}
				}
			}
		}
	}

	// Handle shape editing drag
	if (c->editing_shape && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
		if (c->selected_idx >= 0 && c->selected_idx < (int)c->shapes.size()) {
			em_shape_t* s = &c->shapes[c->selected_idx];
			if (s->type == EM_SHAPE_RECT) {
				// Update corner/edge based on handle
				if (c->edit_handle == 0 || c->edit_handle == 1 || c->edit_handle == 2 || c->edit_handle == 7) {
					s->p0.y = mouse_canvas.y;
				}
				if (c->edit_handle == 4 || c->edit_handle == 5 || c->edit_handle == 6 || c->edit_handle == 3) {
					s->p1.y = mouse_canvas.y;
				}
				if (c->edit_handle == 0 || c->edit_handle == 6 || c->edit_handle == 7 || c->edit_handle == 1) {
					s->p0.x = mouse_canvas.x;
				}
				if (c->edit_handle == 2 || c->edit_handle == 3 || c->edit_handle == 4 || c->edit_handle == 5) {
					s->p1.x = mouse_canvas.x;
				}
			} else if (s->type == EM_SHAPE_CIRCLE) {
				// Update radius
				float dx = mouse_canvas.x - s->p0.x;
				float dy = mouse_canvas.y - s->p0.y;
				s->p1.x = sqrtf(dx*dx + dy*dy);
			}
		}
	}

	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
		c->editing_shape = false;
		c->edit_handle = -1;
	}

	// finish rect/circle on mouse release
	if (c->drawing && c->draw_mode != EM_SHAPE_POLYGON) {
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
			ImVec2 end = mouse_canvas;
			float dx = end.x - c->draw_start.x;
			float dy = end.y - c->draw_start.y;
			bool big_enough = (dx*dx + dy*dy) > (4.0f / (c->grid.scale * c->grid.scale));
			if (big_enough) {
				em_shape_t s;
				s.type     = c->draw_mode;
				s.material = c->active_material;
				s.selected = false;
				if (c->draw_mode == EM_SHAPE_RECT) {
					s.p0 = c->draw_start;
					s.p1 = end;
				} else { // circle
					s.p0 = c->draw_start;
					float r = sqrtf(dx*dx + dy*dy);
					s.p1 = ImVec2(r, 0);
				}
				c->shapes.push_back(s);
			}
			c->drawing = false;
		}
	}

	// cancel polygon with Escape
	if (c->draw_mode == EM_SHAPE_POLYGON && ImGui::IsKeyPressed(ImGuiKey_Escape)) {
		c->poly_wip.clear();
		c->drawing = false;
	}

	// ---- Render -----------------------------------------------------------
	ImDrawList* dl = ImGui::GetWindowDrawList();
	dl->PushClipRect(ImVec2(0, 0), canvas_size, true);

	// background
	dl->AddRectFilled(ImVec2(0, 0), canvas_size, IM_COL32(18, 22, 30, 255));

	drawgrid(dl, c);

	// FDTD field visualization (behind shapes)
	if (c->fdtd.initialized && c->fdtd.running) {
		fdtd_draw_field(dl, c, canvas_size);
	}

	// committed shapes
	for (int i = 0; i < (int)c->shapes.size(); i++) {

		// Draw shapes
		ImU32 fill   = ImGui::ColorConvertFloat4ToU32(c->shapes[i].material.color);
		ImU32 border = i == c->selected_idx
			? IM_COL32(255, 220, 60, 255)
			: IM_COL32(200, 200, 200, 200);
		float thick = i == c->selected_idx ? 2.5f : 1.5f;

		switch (c->shapes[i].type) {
			case EM_SHAPE_RECT: {
				ImVec2 a = ImVec2(
					c->grid.offset[0] + c->shapes[i].p0.x * c->grid.scale,
					c->grid.offset[1] + c->shapes[i].p0.y * c->grid.scale
				);
				ImVec2 b = ImVec2(
					c->grid.offset[0] + c->shapes[i].p1.x * c->grid.scale,
					c->grid.offset[1] + c->shapes[i].p1.y * c->grid.scale
				);

				dl->AddRectFilled(a, b, fill);
				dl->AddRect(a, b, border, 0.0f, 0, thick);
				dl->AddText(a, IM_COL32(255,255,255,200), c->shapes[i].material.name);
				break;
			}
			case EM_SHAPE_CIRCLE: {
				double r   = c->shapes[i].p1.x * c->grid.scale;
				ImVec2 ctr = ImVec2(
					c->grid.offset[0] + c->shapes[i].p0.x * c->grid.scale,
					c->grid.offset[1] + c->shapes[i].p0.y * c->grid.scale
				);
				dl->AddCircleFilled(ctr, r, fill);
				dl->AddCircle(ctr, r, border, 64, thick);
				dl->AddText(ImVec2(ctr.x - 20, ctr.y - 7),
							IM_COL32(255,255,255,200), c->shapes[i].material.name);
				break;
			}
			case EM_SHAPE_POLYGON: {
				// TODO: free the src array
				size_t size = c->shapes[i].poly_pts.size();
				if (size < 2) break;
				ImVec2* scr = (ImVec2*) calloc(size, sizeof(double)*2);
				for(size_t j = 0; j < size; j++)
					scr[j] = ImVec2(c->grid.offset[0] + c->shapes[i].poly_pts[j].x * c->grid.scale,
					                c->grid.offset[1] + c->shapes[i].poly_pts[j].y * c->grid.scale);

				dl->AddConvexPolyFilled(scr, size, fill);
				dl->AddPolyline(scr, size, border, ImDrawFlags_Closed, thick);
				dl->AddText(scr[0], IM_COL32(255,255,255,200), c->shapes[i].material.name);
				break;
			}
		}

		// Draw edit handles for selected shape
		if (i == c->selected_idx) {
			if (c->shapes[i].type == EM_SHAPE_RECT) {
				double a[2] = {
					c->grid.offset[0] + c->shapes[i].p0.x * c->grid.scale,
					c->grid.offset[1] + c->shapes[i].p0.y * c->grid.scale
				};
				double b[2] = {
					c->grid.offset[0] + c->shapes[i].p1.x * c->grid.scale,
					c->grid.offset[1] + c->shapes[i].p1.y * c->grid.scale
				};

				double handles[8][2] = {
					{a[0]            , a[1]           },
					{(a[0]+b[0])*0.5f, a[1]           },
					{b[0]            , a[1]           },
					{b[0]            ,(a[1]+b[1])*0.5f},
					{b[0]            , b[1]           },
					{(a[0]+b[0])*0.5f, b[1]           },
					{a[0]            , b[1]           },
					{a[0]            ,(a[1]+b[1])*0.5f},
				};

				for (int i = 0; i < 8; i++)
					dl->AddCircleFilled(ImVec2(handles[i][0], handles[i][1]),
					             5.0f, IM_COL32(255, 255, 255, 255));

			} else if (c->shapes[i].type == EM_SHAPE_CIRCLE) {
				double r = c->shapes[i].p1.x * c->grid.scale;
				double ctr[2] = {
					c->grid.offset[0] + c->shapes[i].p0.x * c->grid.scale,
					c->grid.offset[1] + c->shapes[i].p0.y * c->grid.scale
				};

				double handles[4][2] = {
					{ctr[0] + r, ctr[1]    },
					{ctr[0]    , ctr[1] + r},
					{ctr[0] - r, ctr[1]    },
					{ctr[0]    , ctr[1] - r},
				};

				for (int i = 0; i < 4; i++)
					dl->AddCircleFilled(ImVec2(handles[i][0], handles[i][1]),
					            5.0f, IM_COL32(255, 255, 255, 255));
			}
		}
	}

	// Draw sources
	for (int i = 0; i < (int)c->sources.size(); i++) {
		size_t color = i == c->selected_source_idx ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 100, 100, 255);
		double screen_pos[2] = {
			c->grid.offset[0] + c->sources[i].pos.x * c->grid.scale,
			c->grid.offset[1] + c->sources[i].pos.y * c->grid.scale
		};

		if (c->sources[i].type == EM_SOURCE_POINT) {
			// Draw as pulsing circle
			double r = 8.0f + 3.0f * sinf(ImGui::GetTime() * 4.0f);
			dl->AddCircleFilled(ImVec2(screen_pos[0], screen_pos[1]), r, color);
			dl->AddCircle(ImVec2(screen_pos[0], screen_pos[1]), r + 2,
			              IM_COL32(255, 255, 255, 200), 16, 2.0f);
			// Label
			dl->AddText(ImVec2(screen_pos[0] + 12, screen_pos[1] - 7), 
			            IM_COL32(255, 255, 255, 255), c->sources[i].name);
		} else if (c->sources[i].type == EM_SOURCE_PLANE) {
			// Draw as arrow indicating direction
			double len = 30.0f;
			double dx = cosf(c->sources[i].angle) * len;
			double dy = sinf(c->sources[i].angle) * len;
			ImVec2 end(screen_pos[0] + dx, screen_pos[1] + dy);
			dl->AddLine(ImVec2(screen_pos[0], screen_pos[1]), end, color, 3.0f);
			// Arrowhead
			double angle = c->sources[i].angle + M_PI;
			ImVec2 a1(end.x + 10*cosf(angle + 0.5f), end.y + 10*sinf(angle + 0.5f));
			ImVec2 a2(end.x + 10*cosf(angle - 0.5f), end.y + 10*sinf(angle - 0.5f));
			dl->AddTriangleFilled(end, a1, a2, color);
			dl->AddText(ImVec2(screen_pos[0] + 12, screen_pos[1] - 7), 
			            IM_COL32(255, 255, 255, 255), c->sources[i].name);
		}
	}

	// Draw probes
	for (int i = 0; i < (int)c->probes.size(); i++) {
		size_t color = i == c->selected_probe_idx ? IM_COL32(255, 255, 0, 255) : IM_COL32(100, 255, 100, 255);
		double screen_pos[2] = {
			c->grid.offset[0] + c->probes[i].pos.x * c->grid.scale,
			c->grid.offset[1] + c->probes[i].pos.y * c->grid.scale
		};

		// Draw as square
		float sz = 15.0f;
		dl->AddRectFilled(ImVec2(screen_pos[0] - sz, screen_pos[1] - sz),
		                  ImVec2(screen_pos[0] + sz, screen_pos[1] + sz), color);
		dl->AddRect(ImVec2(screen_pos[0] - sz - 1, screen_pos[1] - sz - 1),
		            ImVec2(screen_pos[0] + sz + 1, screen_pos[1] + sz + 1),
		            IM_COL32(255, 255, 255, 200), 0, 0, 2.0f);

		// Label with value
		char label[64];
		snprintf(label, 64, "%s: %.2e", c->probes[i].name, c->probes[i].value);
		dl->AddText(ImVec2(screen_pos[0] + 10, screen_pos[1] - 7), 
		        IM_COL32(255, 255, 255, 255), label);
	}

	// preview while drawing
	if (c->drawing) {
		ImU32 prev_fill   = IM_COL32(100, 200, 255, 60);
		ImU32 prev_border = IM_COL32(100, 200, 255, 200);

		if (c->draw_mode == EM_SHAPE_RECT) {
			ImVec2 a = ImVec2(
				c->grid.offset[0] + c->draw_start.x * c->grid.scale,
				c->grid.offset[1] + c->draw_start.y * c->grid.scale
			);
			ImVec2 b = ImVec2(
				c->grid.offset[0] + mouse_canvas.x * c->grid.scale,
				c->grid.offset[1] + mouse_canvas.y * c->grid.scale
			);
			dl->AddRectFilled(a, b, prev_fill);
			dl->AddRect(a, b, prev_border, 0, 0, 1.5f);
		} else if (c->draw_mode == EM_SHAPE_CIRCLE) {
			ImVec2 ctr = ImVec2(
				c->grid.offset[0] + c->draw_start.x * c->grid.scale,
				c->grid.offset[1] + c->draw_start.y * c->grid.scale
			);
			double dx = mouse_canvas.x - c->draw_start.x, dy = mouse_canvas.y - c->draw_start.y;
			double r = sqrtf(dx*dx + dy*dy) * c->grid.scale;
			dl->AddCircleFilled(ctr, r, prev_fill);
			dl->AddCircle(ctr, r, prev_border, 64, 1.5f);
		} else if (c->draw_mode == EM_SHAPE_POLYGON) {
			// draw committed vertices + line to cursor
			for (size_t i = 0; i + 1 < c->poly_wip.size(); i++)
				dl->AddLine(
					ImVec2(
						c->grid.offset[0] + c->poly_wip[i].x * c->grid.scale,
						c->grid.offset[1] + c->poly_wip[i].y * c->grid.scale),
					ImVec2(
						c->grid.offset[0] + mouse_canvas.x * c->grid.scale,
						c->grid.offset[1] + mouse_canvas.y * c->grid.scale),
					prev_border, 1.5f);

			if (!c->poly_wip.empty())
				dl->AddLine(
					ImVec2(
						c->grid.offset[0] + c->poly_wip.back().x * c->grid.scale,
						c->grid.offset[1] + c->poly_wip.back().y * c->grid.scale),
					ImVec2(
						c->grid.offset[0] + mouse_canvas.x * c->grid.scale,
						c->grid.offset[1] + mouse_canvas.y * c->grid.scale),
					IM_COL32(100,200,255,120), 1.5f);

			// vertex dots
			for(size_t j = 0; j < c->poly_wip.size(); j++)
				dl->AddCircleFilled(ImVec2(
					c->grid.offset[0] + c->poly_wip[j].x * c->grid.scale,
					c->grid.offset[1] + c->poly_wip[j].y * c->grid.scale),
				4.0f, IM_COL32(100,200,255,255));
		}
	}

	dl->PopClipRect();
	ImGui::End();
}

inline void Dockspace(GLFWwindow* window, editor_t* editor, app_mode_t mode){
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

	if(mode == MODE_CIRCUIT){
		ImGui::Begin("Simulation", NULL, ImGuiWindowFlags_None);
		ImGui::Text("Text");
		ImGui::Text("(%.1f FPS)", ImGui::GetIO().Framerate);
		ImGui::End();

		ImGui::Begin("Properties", NULL, ImGuiWindowFlags_None);
		ImGui::End();

		ImGui::Begin("Explorer", NULL, ImGuiWindowFlags_None);
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

		ImGui::Begin("Diagram", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		processInput(window, editor);
		ImDrawList* drawlist = ImGui::GetWindowDrawList();
		drawgrid(drawlist, editor);
		draw_comps(drawlist, editor);
		drawlinks(drawlist, editor);
		showeditormenu(editor);
		show_comp_menu(editor);
		ImGui::End();
	} else {
		em_editor_panel(window, editor, dockspace_id);
	}
}

#endif //GUI_H_
