﻿/******************************************************************************/
template <int i>
void PanelImageEditor::LightEnabled(Params &p, C Str &t) {
    p.lights[i].enabled = TextBool(t);
    p.base.lights[i].enabled.getUTC();
}
template <int i>
void PanelImageEditor::LightAngle(Params &p, C Str &t) {
    p.lights[i].angle = TextVec2(t);
    p.base.lights[i].angle.getUTC();
}
template <int i>
void PanelImageEditor::LightIntensity(Params &p, C Str &t) {
    p.lights[i].intensity = TextFlt(t);
    p.base.lights[i].intensity.getUTC();
}
template <int i>
void PanelImageEditor::LightBack(Params &p, C Str &t) {
    p.lights[i].back = TextFlt(t);
    p.base.lights[i].back.getUTC();
}
template <int i>
void PanelImageEditor::LightHighlight(Params &p, C Str &t) {
    p.lights[i].highlight = TextFlt(t);
    p.base.lights[i].highlight.getUTC();
}
template <int i>
void PanelImageEditor::LightHighlightCut(Params &p, C Str &t) {
    p.lights[i].highlight_cut = TextFlt(t);
    p.base.lights[i].highlight_cut.getUTC();
}
template <int i>
void PanelImageEditor::LightSpecular(Params &p, C Str &t) {
    p.lights[i].specular = TextFlt(t);
    p.base.lights[i].specular.getUTC();
}
template <int i>
void PanelImageEditor::LightSpecularBack(Params &p, C Str &t) {
    p.lights[i].specular_back = TextFlt(t);
    p.base.lights[i].specular_back.getUTC();
}
template <int i>
void PanelImageEditor::LightSpecularExp(Params &p, C Str &t) {
    p.lights[i].specular_exp = TextFlt(t);
    p.base.lights[i].specular_exp.getUTC();
}
template <int i>
void PanelImageEditor::LightSpecularHighlight(Params &p, C Str &t) {
    p.lights[i].specular_highlight = TextFlt(t);
    p.base.lights[i].specular_highlight.getUTC();
}
template <int i>
void PanelImageEditor::LightSpecularHighlightCut(Params &p, C Str &t) {
    p.lights[i].specular_highlight_cut = TextFlt(t);
    p.base.lights[i].specular_highlight_cut.getUTC();
}
template <int i>
void PanelImageEditor::SectionSize(Params &p, C Str &t) {
    p.sections[i].size = TextFlt(t);
    p.base.sections[i].size.getUTC();
}
template <int i>
void PanelImageEditor::SectionTopOffset(Params &p, C Str &t) {
    p.sections[i].top_offset = TextFlt(t);
    p.base.sections[i].top_offset.getUTC();
}
template <int i>
void PanelImageEditor::SectionRoundDepth(Params &p, C Str &t) {
    p.sections[i].round_depth = TextFlt(t);
    p.base.sections[i].round_depth.getUTC();
}
template <int i>
void PanelImageEditor::SectionOuterDepth(Params &p, C Str &t) {
    p.sections[i].outer_depth = TextFlt(t);
    p.base.sections[i].outer_depth.getUTC();
}
template <int i>
void PanelImageEditor::SectionInnerDepth(Params &p, C Str &t) {
    p.sections[i].inner_depth = TextFlt(t);
    p.base.sections[i].inner_depth.getUTC();
}
template <int i>
void PanelImageEditor::SectionInnerDistance(Params &p, C Str &t) {
    p.sections[i].inner_distance = TextFlt(t);
    p.base.sections[i].inner_distance.getUTC();
}
template <int i>
void PanelImageEditor::SectionSmoothDepth(Params &p, C Str &t) {
    p.sections[i].smooth_depth = TextVec2(t);
    p.base.sections[i].smooth_depth.getUTC();
}
template <int i>
void PanelImageEditor::SectionSpecular(Params &p, C Str &t) {
    p.sections[i].specular = TextFlt(t);
    p.base.sections[i].specular.getUTC();
}
template <int i>
void PanelImageEditor::SectionColor(Params &p, C Str &t) {
    p.sections[i].color = TextVec4(t);
    p.base.sections[i].color.getUTC();
}
template <int i>
void PanelImageEditor::SectionOuterColor(Params &p, C Str &t) {
    p.sections[i].outer_color = TextVec4(t);
    p.base.sections[i].outer_color.getUTC();
}
template <int i>
void PanelImageEditor::SectionInnerColor(Params &p, C Str &t) {
    p.sections[i].inner_color = TextVec4(t);
    p.base.sections[i].inner_color.getUTC();
}
template <int i>
void PanelImageEditor::SectionColorTop(Params &p, C Str &t) {
    p.sections[i].color_top = TextVec4(t);
    p.base.sections[i].color_top.getUTC();
}
template <int i>
void PanelImageEditor::SectionColorBottom(Params &p, C Str &t) {
    p.sections[i].color_bottom = TextVec4(t);
    p.base.sections[i].color_bottom.getUTC();
}
template <int i>
void PanelImageEditor::SectionColorLeft(Params &p, C Str &t) {
    p.sections[i].color_left = TextVec4(t);
    p.base.sections[i].color_left.getUTC();
}
template <int i>
void PanelImageEditor::SectionColorRight(Params &p, C Str &t) {
    p.sections[i].color_right = TextVec4(t);
    p.base.sections[i].color_right.getUTC();
}
template <int i>
void PanelImageEditor::SectionOuterBorderColor(Params &p, C Str &t) {
    p.sections[i].outer_border_color = TextVec4(t);
    p.base.sections[i].outer_border_color.getUTC();
}
template <int i>
void PanelImageEditor::SectionInnerBorderColor(Params &p, C Str &t) {
    p.sections[i].inner_border_color = TextVec4(t);
    p.base.sections[i].inner_border_color.getUTC();
}
template <int i>
void PanelImageEditor::SectionPrevBorderColor(Params &p, C Str &t) {
    p.sections[i].prev_border_color = TextVec4(t);
    p.base.sections[i].prev_border_color.getUTC();
}
template <int i>
Str PanelImageEditor::SectionDepthOverlay(C Params &p) { return Proj.elmFullName(p.base.sections[i].depth_overlay_id); }
template <int i>
void PanelImageEditor::SectionDepthOverlay(Params &p, C Str &t) {
    p.base.sections[i].depth_overlay_id = Proj.findElmImageID(t);
    p.base.sections[i].depth_overlay.getUTC();
}
template <int i>
void PanelImageEditor::SectionDepthOverlayBlur(Params &p, C Str &t) {
    p.sections[i].depth_overlay_params.blur = TextInt(t);
    p.base.sections[i].depth_overlay_params.blur.getUTC();
}
template <int i>
void PanelImageEditor::SectionDepthOverlayBlurClamp(Params &p, C Str &t) {
    p.sections[i].depth_overlay_params.blur_clamp = TextBool(t);
    p.base.sections[i].depth_overlay_params.blur_clamp.getUTC();
}
template <int i>
void PanelImageEditor::SectionDepthOverlayUVScale(Params &p, C Str &t) {
    p.sections[i].depth_overlay_params.uv_scale = TextFlt(t);
    p.base.sections[i].depth_overlay_params.uv_scale.getUTC();
}
template <int i>
void PanelImageEditor::SectionDepthOverlayUVOffset(Params &p, C Str &t) {
    p.sections[i].depth_overlay_params.uv_offset = TextVec2(t);
    p.base.sections[i].depth_overlay_params.uv_offset.getUTC();
}
template <int i>
void PanelImageEditor::SectionDepthOverlayIntensity(Params &p, C Str &t) {
    p.sections[i].depth_overlay_params.intensity = TextFlt(t);
    p.base.sections[i].depth_overlay_params.intensity.getUTC();
}
template <int i>
void PanelImageEditor::SectionDepthOverlayMode(Params &p, C Str &t) {
    p.sections[i].depth_overlay_params.mode = (PanelImageParams::ImageParams::MODE)TextInt(t);
    p.base.sections[i].depth_overlay_params.mode.getUTC();
}
template <int i>
void PanelImageEditor::SectionDepthNoiseBlur(Params &p, C Str &t) {
    p.sections[i].depth_noise.blur = TextInt(t);
    p.base.sections[i].depth_noise.blur.getUTC();
}
template <int i>
void PanelImageEditor::SectionDepthNoiseUVScale(Params &p, C Str &t) {
    p.sections[i].depth_noise.uv_scale = TextFlt(t);
    p.base.sections[i].depth_noise.uv_scale.getUTC();
}
template <int i>
void PanelImageEditor::SectionDepthNoiseIntensity(Params &p, C Str &t) {
    p.sections[i].depth_noise.intensity = TextFlt(t);
    p.base.sections[i].depth_noise.intensity.getUTC();
}
template <int i>
void PanelImageEditor::SectionDepthNoiseMode(Params &p, C Str &t) {
    p.sections[i].depth_noise.mode = (PanelImageParams::ImageParams::MODE)TextInt(t);
    p.base.sections[i].depth_noise.mode.getUTC();
}
template <int i>
Str PanelImageEditor::SectionColorOverlay(C Params &p) { return Proj.elmFullName(p.base.sections[i].color_overlay_id); }
template <int i>
void PanelImageEditor::SectionColorOverlay(Params &p, C Str &t) {
    p.base.sections[i].color_overlay_id = Proj.findElmImageID(t);
    p.base.sections[i].color_overlay.getUTC();
}
template <int i>
void PanelImageEditor::SectionColorOverlayBlur(Params &p, C Str &t) {
    p.sections[i].color_overlay_params.blur = TextInt(t);
    p.base.sections[i].color_overlay_params.blur.getUTC();
}
template <int i>
void PanelImageEditor::SectionColorOverlayBlurClamp(Params &p, C Str &t) {
    p.sections[i].color_overlay_params.blur_clamp = TextBool(t);
    p.base.sections[i].color_overlay_params.blur_clamp.getUTC();
}
template <int i>
void PanelImageEditor::SectionColorOverlayUVScale(Params &p, C Str &t) {
    p.sections[i].color_overlay_params.uv_scale = TextFlt(t);
    p.base.sections[i].color_overlay_params.uv_scale.getUTC();
}
template <int i>
void PanelImageEditor::SectionColorOverlayUVOffset(Params &p, C Str &t) {
    p.sections[i].color_overlay_params.uv_offset = TextVec2(t);
    p.base.sections[i].color_overlay_params.uv_offset.getUTC();
}
template <int i>
void PanelImageEditor::SectionColorOverlayUVWarp(Params &p, C Str &t) {
    p.sections[i].color_overlay_params.uv_warp = TextFlt(t);
    p.base.sections[i].color_overlay_params.uv_warp.getUTC();
}
template <int i>
void PanelImageEditor::SectionColorOverlayIntensity(Params &p, C Str &t) {
    p.sections[i].color_overlay_params.intensity = TextFlt(t);
    p.base.sections[i].color_overlay_params.intensity.getUTC();
}
template <int i>
void PanelImageEditor::SectionColorOverlayMode(Params &p, C Str &t) {
    p.sections[i].color_overlay_params.mode = (PanelImageParams::ImageParams::MODE)TextInt(t);
    p.base.sections[i].color_overlay_params.mode.getUTC();
}
template <int i>
void PanelImageEditor::SectionColorNoiseBlur(Params &p, C Str &t) {
    p.sections[i].color_noise.blur = TextInt(t);
    p.base.sections[i].color_noise.blur.getUTC();
}
template <int i>
void PanelImageEditor::SectionColorNoiseUVScale(Params &p, C Str &t) {
    p.sections[i].color_noise.uv_scale = TextFlt(t);
    p.base.sections[i].color_noise.uv_scale.getUTC();
}
template <int i>
void PanelImageEditor::SectionColorNoiseUVWarp(Params &p, C Str &t) {
    p.sections[i].color_noise.uv_warp = TextFlt(t);
    p.base.sections[i].color_noise.uv_warp.getUTC();
}
template <int i>
void PanelImageEditor::SectionColorNoiseIntensity(Params &p, C Str &t) {
    p.sections[i].color_noise.intensity = TextFlt(t);
    p.base.sections[i].color_noise.intensity.getUTC();
}
template <int i>
void PanelImageEditor::SectionColorNoiseMode(Params &p, C Str &t) {
    p.sections[i].color_noise.mode = (PanelImageParams::ImageParams::MODE)TextInt(t);
    p.base.sections[i].color_noise.mode.getUTC();
}
template <int i>
Str PanelImageEditor::SectionReflection(C Params &p) { return Proj.elmFullName(p.base.sections[i].reflection_id); }
template <int i>
void PanelImageEditor::SectionReflection(Params &p, C Str &t) {
    p.base.sections[i].reflection_id = Proj.findElmImageID(t);
    p.base.sections[i].reflection.getUTC();
}
template <int i>
void PanelImageEditor::SectionReflectionIntensity(Params &p, C Str &t) {
    p.sections[i].reflection_intensity = TextFlt(t);
    p.base.sections[i].reflection_intensity.getUTC();
}
/******************************************************************************/
