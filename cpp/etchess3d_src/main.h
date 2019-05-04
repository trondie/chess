#ifndef __MAIN_H__
#define __MAIN_H__
class EE_Scene;
class EE_GeometryInstance;
class EE_Geometry;

EE_Scene *get_scene();
vector<EE_GeometryInstance *> *get_all_instances();
vector<EE_Geometry *> *get_all_geometries();
void *get_xml_document();

void render_sphere( EE_GeometryInstance *instance );
void render_box( EE_GeometryInstance *instance );
#endif /*  __MAIN_H__ */