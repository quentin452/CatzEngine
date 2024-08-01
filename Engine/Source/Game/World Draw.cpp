/******************************************************************************/
#include "stdafx.h"
namespace EE {
namespace Game {
#define EPS_GRASS_RANGE 0.01f
/******************************************************************************
static void AreaDrawState(Cell<Game::Area> &cell, Ptr)
{
   (cell.xy().vec2()*0.03).draw(cell().state ? ((cell().state==2) ? GREEN : RED) : BLACK);
}
/******************************************************************************/
inline void Area::drawObjAndTerrain() {
    PROFILE_START("Area::drawObjAndTerrain()")
    // first process objects before terrain, so they will be first on the list (if possible), objects don't use EarlyZ, so terrain will always be displayed as first in the EarlyZ stage
    REPA(_objs) {
        Obj &obj = *_objs[i];
        if (UInt modes = obj.drawPrepare())
            if (Renderer.firstPass()) {
                if (modes & IndexToFlag(RM_OVERLAY))
                    OverlayObjects.add(&obj);
                if (modes & IndexToFlag(RM_BLEND))
                    BlendInstances.add(obj);
                if (modes & IndexToFlag(RM_PALETTE))
                    PaletteObjects.add(&obj);
                if (modes & IndexToFlag(RM_PALETTE1))
                    Palette1Objects.add(&obj);
                if (modes & IndexToFlag(RM_OPAQUE))
                    OpaqueObjects.add(&obj);
                if (modes & IndexToFlag(RM_EMISSIVE))
                    EmissiveObjects.add(&obj);
                if (modes & IndexToFlag(RM_OUTLINE))
                    OutlineObjects.add(&obj);
                if (modes & IndexToFlag(RM_BEHIND))
                    BehindObjects.add(&obj);
            }
    }

    // data
    if (_data) {
        if (UInt modes = _data->customDrawPrepare())
            if (Renderer.firstPass()) // custom
            {
                if (modes & IndexToFlag(RM_BLEND))
                    BlendInstances.add(*_data);
                if (modes & IndexToFlag(RM_PALETTE))
                    PaletteAreas.add(_data);
                if (modes & IndexToFlag(RM_PALETTE1))
                    Palette1Areas.add(_data);
            }

        // terrain
        {
            Bool fully_inside;
            SetStencilValue(true);

            // terrain objects
            {
                C Memc<Data::TerrainObj> &to = _data->terrain_objs;
                if (to.elms() && Frustum(_data->terrain_objs_ext, fully_inside))
                    REPA(to) {
                        C Data::TerrainObj &obj = to[i];
                        if (C MeshPtr &mesh = obj.obj->mesh())
                            if (fully_inside || Frustum(*mesh, obj.matrix)) {
                                SetVariation(obj.mesh_variation);
                                mesh->draw(obj.matrix);
                            }
                    }
            }

            // grass objects
            if (D.grassRange() > EPS_GRASS_RANGE) {
                C Memc<Data::GrassObj> &fo = _data->foliage_objs;
                C FrustumClass &frustum = world()->_frustum_grass;
                if (fo.elms() && frustum(_data->foliage_objs_ext, fully_inside)) {
                    UInt density = RoundPos(D.grassDensity() * 256);
                    REPA(fo) {
                        C Data::GrassObj &obj = fo[i];
                        if (C MeshPtr &mesh = obj.mesh) {
                            Ball ball = mesh->ext;
                            SetVariation(obj.mesh_variation);

                            REP((obj.instances.elms() * density) >> 8) {
                                C Data::GrassObj::Instance &instance = obj.instances[i];
                                Flt dist2 = Dist2(instance.matrix.pos, ActiveCam.matrix.pos);
                                if (dist2 < D._grass_range_sqr && (fully_inside || frustum(ball * instance.matrix))) {
                                    if (!obj.shrink)
                                        mesh->draw(instance.matrix);
                                    else {
                                        Flt f = dist2 / D._grass_range_sqr;
                                        if (f > Sqr(0.9f)) {
                                            auto shrinked = instance.matrix;
                                            mesh->draw(shrinked.scaleOrn(LerpR(1.0f, Sqr(0.9f), f)));
                                        } else {
                                            mesh->draw(instance.matrix);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            SetVariation();

            // terrain mesh
            SetEarlyZ(WorldManager::use_early_z_for_terrain);
            _data->mesh.draw();
            SetEarlyZ();

            SetStencilValue();
        }
    }
    PROFILE_STOP("Area::drawObjAndTerrain()")
}
inline void Area::drawTerrainShadow() {
    PROFILE_START("Area::drawTerrainShadow()")
    if (_data) {
        Bool fully_inside;

        // terrain objects
        {
            Memc<Data::TerrainObj> &to = _data->terrain_objs;
            if (to.elms() && Frustum(_data->terrain_objs_ext, fully_inside))
                REPA(to) {
                    C Data::TerrainObj &obj = to[i];
                    if (obj.obj->mesh() && (fully_inside || Frustum(*obj.obj->mesh(), obj.matrix))) {
                        SetVariation(obj.mesh_variation);
                        obj.obj->mesh()->drawShadow(obj.matrix);
                    }
                }
        }

        // grass objects
        if (D.grassShadow() && D.grassRange() > EPS_GRASS_RANGE) {
            C Memc<Data::GrassObj> &fo = _data->foliage_objs;
            if (fo.elms() && Frustum(_data->foliage_objs_ext, fully_inside)) {
                UInt density = RoundPos(D.grassDensity() * 256);
                REPA(fo) {
                    C Data::GrassObj &obj = fo[i];
                    if (obj.mesh) {
                        Ball ball = obj.mesh->ext;
                        SetVariation(obj.mesh_variation);

                        REP((obj.instances.elms() * density) >> 8) {
                            C Data::GrassObj::Instance &instance = obj.instances[i];
                            if (Dist2(instance.matrix.pos, ActiveCam.matrix.pos) < D._grass_range_sqr)
                                if (fully_inside || Frustum(ball * instance.matrix))
                                    obj.mesh->drawShadow(instance.matrix);
                        }
                    }
                }
            }
        }
        SetVariation();

        // terrain mesh
        data()->mesh.drawShadow();

        // custom
        data()->customDrawShadow();
    }
    PROFILE_STOP("Area::drawTerrainShadow()")
}
inline void Area::drawObjShadow() {
    REPAO(_objs)->drawShadow();
}
/******************************************************************************/
inline void Area::drawOverlay() {
    PROFILE_START("Area::drawOverlay()")
    if (_data) {
        // mesh overlays
        Memc<MeshOverlay> &mesh_overlays = _data->mesh_overlays;
        if (mesh_overlays.elms()) {
            SetOneMatrix();
            FREPA(mesh_overlays) {
                MeshOverlay &mo = mesh_overlays[i];
                if (Frustum(mo._ext))
                    mo.draw();
            }
        }

        // decals
        Memc<Decal> &decals = _data->decals;
        FREPAO(decals).drawStatic();
    }
    PROFILE_STOP("Area::drawOverlay()")
}
/******************************************************************************/
void WorldManager::draw() {
    PROFILE_START("WorldManager::draw()")
    switch (Renderer()) {
    case RM_PREPARE: {
        // objects and terrain
        if (Renderer.firstPass())
            _frustum_grass.set(Min(D._view_active.range, D.grassRange()), D._view_active.fov, CamMatrix);
        if (Renderer.firstPass() || CurrentLight.type == LIGHT_DIR) // directional lights cover entire screen, so we don't have to recalculate visible areas and just use '_area_draw'
        {                                                           // '_area_draw' were calculated in MODE_UNDERWATER
            FREPAO(_area_draw)->drawObjAndTerrain();                // draw in order
        } else {                                                    // recalculate visible areas for secondary local lights because usually there will be fewer of them
            areaSetVisibility(_area_draw_secondary, false);
            FREPAO(_area_draw_secondary)->drawObjAndTerrain(); // draw in order
        }
    } break;

    case RM_SHADOW: {
        // set areas for drawing, call this first
        areaSetVisibility(_area_draw_shadow, false);

        // objects
        FREPAO(_area_draw_shadow)->drawObjShadow(); // draw in order

        // terrain
        FREPAO(_area_draw_shadow)->drawTerrainShadow(); // draw in order
    } break;

    // overlays and decals
    case RM_OVERLAY: {
        FREPAO(_area_draw)->drawOverlay(); // draw in order

        // mesh overlays
        if (_mesh_overlays.elms()) {
            SetOneMatrix();
            FREPA(_mesh_overlays) {
                MeshOverlay2 &mo = _mesh_overlays[i];
                if (Frustum(mo._ext))
                    mo.draw(Sat(mo.time));
            }
        }

        // decals
        FREPA(_decals) {
            Decal2 &decal = _decals[i];
            Flt color_w = decal.color.w;
            decal.color.w *= Sat(decal.time);
            decal.drawStatic();
            decal.color.w = color_w;
        }
    } break;

    // water
    case RM_WATER:
        switch (Water._mode) {
        case WaterClass::MODE_DRAW: {
            FREPA(_area_draw) // draw in order
            {
                if (Area::Data *data = _area_draw[i]->_data)
                    REPAO(data->waters).draw();
            }
        } break;

        case WaterClass::MODE_REFLECTION: {
            FREPA(_area_draw) // draw in order
            {
                if (Renderer._mirror_want && Renderer._mirror_priority >= 0)
                    break; // if already found requested reflection then stop searching
                if (Area::Data *data = _area_draw[i]->_data)
                    REPAO(data->waters).draw();
            }
        } break;

        case WaterClass::MODE_UNDERWATER: // when testing if underwater use only water areas from the area where camera is located
        {
            areaSetVisibility(_area_draw, true); // !! this assumes that RM_WATER+MODE_UNDERWATER are called as the first thing during rendering, we call it only once and ignore for reflections, assuming they will always give the same areas !!
            if (Area *area = areaActive(worldToArea(ActiveCam.matrix.pos)))
                if (Area::Data *data = area->data())
                    REPAO(data->waters).draw();
        } break;
        }
        break;
    }
    PROFILE_STOP("WorldManager::draw()")
}
void WorldManager::drawDrawnAreas(C Color &color, C Color &color_shd) {
    PROFILE_START("WorldManager::drawDrawnAreas(C Color &color, C Color &color_shd)")
    Flt scale = 0.003f;
    if (color.a) {
        VI.color(color);
        REPA(_area_draw)
        VI.rect(Rect_LD(_area_draw[i]->xz(), 1, 1).extend(-0.03f) * areaSize() * scale);
        VI.end();
    }
    if (color_shd.a) {
        VI.color(color_shd);
        REPA(_area_draw_shadow)
        VI.rect(Rect_LD(_area_draw_shadow[i]->xz(), 1, 1).extend(-0.03f) * areaSize() * scale);
        VI.end();
    }
    (ActiveCam.matrix.pos.xz() * scale).draw(WHITE);
    REP(Frustum.edges)
    D.line(WHITE, Frustum.point[Frustum.edge[i].x].xz() * scale, Frustum.point[Frustum.edge[i].y].xz() * scale);
    PROFILE_STOP("WorldManager::drawDrawnAreas(C Color &color, C Color &color_shd)")
}
/******************************************************************************/
} // namespace Game
} // namespace EE
/******************************************************************************/
