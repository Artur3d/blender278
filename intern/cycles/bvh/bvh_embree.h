/*
 * Modifications Copyright 2017, Blender Foundation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __BVH_EMBREE_H__
#define __BVH_EMBREE_H__

#ifdef WITH_EMBREE

#include "bvh.h"
#include "bvh_params.h"

#include "util/util_types.h"
#include "util/util_vector.h"
#include "util/util_thread.h"

#include "embree2/rtcore.h"
#include "embree2/rtcore_scene.h"

CCL_NAMESPACE_BEGIN

class Mesh;

class BVHEmbree : public BVH
{
public:
	void build(Progress& progress, Stats *stats);
	virtual ~BVHEmbree();
	RTCScene scene;

	void mem_monitor(ssize_t mem);
protected:
	/* constructor */
	friend class BVH;
	BVHEmbree(const BVHParams& params, const vector<Object*>& objects);

	virtual void pack_nodes(const BVHNode *root);
	virtual void refit_nodes();

	unsigned add_object(Object *ob, int i);
	unsigned add_instance(Object *ob, int i);
	unsigned add_curves(Mesh *mesh, int i);
	unsigned add_triangles(Mesh *mesh, int i);

	ssize_t mem_used;

	void add_delayed_delete_scene(RTCScene scene) { delayed_delete_scenes.push_back(scene); }
	BVHEmbree *top_level;
private:
	void delete_rtcScene();
	void update_tri_vertex_buffer(unsigned geom_id, const Mesh* mesh);
	void update_curve_vertex_buffer(unsigned geom_id, const Mesh* mesh);

	static RTCDevice rtc_shared_device;
	static int rtc_shared_users;
	static thread_mutex rtc_shared_mutex;

	Stats *stats;
	vector<RTCScene> delayed_delete_scenes;
	int curve_subdivisions;
	bool use_curves, use_ribbons, dynamic_scene;
};

CCL_NAMESPACE_END

#endif /* WITH_EMBREE */

#endif /* __BVH_EMBREE_H__ */
