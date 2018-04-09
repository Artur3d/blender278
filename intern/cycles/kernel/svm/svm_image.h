/*
 * Copyright 2011-2013 Blender Foundation
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

#ifdef __OIIO__
#  include "kernel/kernel_oiio_globals.h"
#  define NEAREST_LOOKUP_PATHS (PATH_RAY_DIFFUSE | PATH_RAY_SHADOW | PATH_RAY_DIFFUSE_ANCESTOR | PATH_RAY_VOLUME_SCATTER | PATH_RAY_AO | PATH_RAY_GLOSSY)
#  define DIFFUSE_BLUR_PATHS (PATH_RAY_DIFFUSE | PATH_RAY_DIFFUSE_ANCESTOR | PATH_RAY_AO)
#endif

CCL_NAMESPACE_BEGIN

/* Float4 textures on various devices. */
#if defined(__KERNEL_CPU__)
#  define TEX_NUM_FLOAT4_IMAGES		TEX_NUM_FLOAT4_CPU
#elif defined(__KERNEL_CUDA__)
#  if __CUDA_ARCH__ < 300
#    define TEX_NUM_FLOAT4_IMAGES	TEX_NUM_FLOAT4_CUDA
#  else
#    define TEX_NUM_FLOAT4_IMAGES	TEX_NUM_FLOAT4_CUDA_KEPLER
#  endif
#else
#  define TEX_NUM_FLOAT4_IMAGES	TEX_NUM_FLOAT4_OPENCL
#endif

ccl_device float4 svm_image_texture(KernelGlobals *kg, int id, float x, float y, differential ds, differential dt, uint srgb, uint use_alpha, int path_flag)
{
#ifdef __KERNEL_CPU__
#  ifdef __KERNEL_SSE2__
	ssef r_ssef;
	float4 &r = (float4 &)r_ssef;
#  else
	float4 r;
#  endif
#  ifdef __OIIO__
	if(kg->oiio && kg->oiio->textures.size() > id && kg->oiio->textures[id].handle) {
		OIIO::TextureOpt options;
		options.swrap = options.twrap = kg->oiio->textures[id].extension;
		options.anisotropic = 8;
		float missingcolor[4] = {TEX_IMAGE_MISSING_R, TEX_IMAGE_MISSING_G, TEX_IMAGE_MISSING_B, TEX_IMAGE_MISSING_A};
		options.missingcolor = missingcolor;
		
		if(path_flag & NEAREST_LOOKUP_PATHS && !(path_flag & PATH_RAY_SINGULAR)) {
			options.interpmode = OIIO::TextureOpt::InterpClosest;
			options.mipmode = OIIO::TextureOpt::MipModeOneLevel;
		}
		else {
			options.interpmode = kg->oiio->textures[id].interpolation;
			options.mipmode = OIIO::TextureOpt::MipModeAniso;
		}		
		
		if(path_flag & DIFFUSE_BLUR_PATHS) {
			options.sblur = options.tblur = kg->oiio->diffuse_blur;
		}
		else if(path_flag & PATH_RAY_GLOSSY) {
			options.sblur = options.tblur = kg->oiio->glossy_blur;
		}
		else {
			options.sblur = options.tblur = 0.0f;
		}
		
		bool success = kg->oiio->tex_sys->texture(kg->oiio->textures[id].handle, (OIIO::TextureSystem::Perthread*)kg->oiio_tdata, options, x, y, ds.dx, ds.dy, dt.dx, dt.dy, 4, (float*)&r);
		if(!success) {
			(void) kg->oiio->tex_sys->geterror();
		} else {
			/* Mip maps are always linear. */
			srgb = !kg->oiio->textures[id].is_linear;
		}
	}
	else
#  endif
	r = kernel_tex_image_interp(id, x, y);
#elif defined(__KERNEL_OPENCL__)
	float4 r = kernel_tex_image_interp(kg, id, x, y);
#else
	float4 r;

#  if __CUDA_ARCH__ < 300
	/* not particularly proud of this massive switch, what are the
	 * alternatives?
	 * - use a single big 1D texture, and do our own lookup/filtering
	 * - group by size and use a 3d texture, performance impact
	 * - group into larger texture with some padding for correct lerp
	 *
	 * also note that cuda has a textures limit (128 for Fermi, 256 for Kepler),
	 * and we cannot use all since we still need some for other storage */

	switch(id) {
		case 0: r = kernel_tex_image_interp(__tex_image_float4_000, x, y); break;
		case 1: r = kernel_tex_image_interp(__tex_image_float4_001, x, y); break;
		case 2: r = kernel_tex_image_interp(__tex_image_float4_002, x, y); break;
		case 3: r = kernel_tex_image_interp(__tex_image_float4_003, x, y); break;
		case 4: r = kernel_tex_image_interp(__tex_image_float4_004, x, y); break;
		case 5: r = kernel_tex_image_interp(__tex_image_byte4_005, x, y); break;
		case 6: r = kernel_tex_image_interp(__tex_image_byte4_006, x, y); break;
		case 7: r = kernel_tex_image_interp(__tex_image_byte4_007, x, y); break;
		case 8: r = kernel_tex_image_interp(__tex_image_byte4_008, x, y); break;
		case 9: r = kernel_tex_image_interp(__tex_image_byte4_009, x, y); break;
		case 10: r = kernel_tex_image_interp(__tex_image_byte4_010, x, y); break;
		case 11: r = kernel_tex_image_interp(__tex_image_byte4_011, x, y); break;
		case 12: r = kernel_tex_image_interp(__tex_image_byte4_012, x, y); break;
		case 13: r = kernel_tex_image_interp(__tex_image_byte4_013, x, y); break;
		case 14: r = kernel_tex_image_interp(__tex_image_byte4_014, x, y); break;
		case 15: r = kernel_tex_image_interp(__tex_image_byte4_015, x, y); break;
		case 16: r = kernel_tex_image_interp(__tex_image_byte4_016, x, y); break;
		case 17: r = kernel_tex_image_interp(__tex_image_byte4_017, x, y); break;
		case 18: r = kernel_tex_image_interp(__tex_image_byte4_018, x, y); break;
		case 19: r = kernel_tex_image_interp(__tex_image_byte4_019, x, y); break;
		case 20: r = kernel_tex_image_interp(__tex_image_byte4_020, x, y); break;
		case 21: r = kernel_tex_image_interp(__tex_image_byte4_021, x, y); break;
		case 22: r = kernel_tex_image_interp(__tex_image_byte4_022, x, y); break;
		case 23: r = kernel_tex_image_interp(__tex_image_byte4_023, x, y); break;
		case 24: r = kernel_tex_image_interp(__tex_image_byte4_024, x, y); break;
		case 25: r = kernel_tex_image_interp(__tex_image_byte4_025, x, y); break;
		case 26: r = kernel_tex_image_interp(__tex_image_byte4_026, x, y); break;
		case 27: r = kernel_tex_image_interp(__tex_image_byte4_027, x, y); break;
		case 28: r = kernel_tex_image_interp(__tex_image_byte4_028, x, y); break;
		case 29: r = kernel_tex_image_interp(__tex_image_byte4_029, x, y); break;
		case 30: r = kernel_tex_image_interp(__tex_image_byte4_030, x, y); break;
		case 31: r = kernel_tex_image_interp(__tex_image_byte4_031, x, y); break;
		case 32: r = kernel_tex_image_interp(__tex_image_byte4_032, x, y); break;
		case 33: r = kernel_tex_image_interp(__tex_image_byte4_033, x, y); break;
		case 34: r = kernel_tex_image_interp(__tex_image_byte4_034, x, y); break;
		case 35: r = kernel_tex_image_interp(__tex_image_byte4_035, x, y); break;
		case 36: r = kernel_tex_image_interp(__tex_image_byte4_036, x, y); break;
		case 37: r = kernel_tex_image_interp(__tex_image_byte4_037, x, y); break;
		case 38: r = kernel_tex_image_interp(__tex_image_byte4_038, x, y); break;
		case 39: r = kernel_tex_image_interp(__tex_image_byte4_039, x, y); break;
		case 40: r = kernel_tex_image_interp(__tex_image_byte4_040, x, y); break;
		case 41: r = kernel_tex_image_interp(__tex_image_byte4_041, x, y); break;
		case 42: r = kernel_tex_image_interp(__tex_image_byte4_042, x, y); break;
		case 43: r = kernel_tex_image_interp(__tex_image_byte4_043, x, y); break;
		case 44: r = kernel_tex_image_interp(__tex_image_byte4_044, x, y); break;
		case 45: r = kernel_tex_image_interp(__tex_image_byte4_045, x, y); break;
		case 46: r = kernel_tex_image_interp(__tex_image_byte4_046, x, y); break;
		case 47: r = kernel_tex_image_interp(__tex_image_byte4_047, x, y); break;
		case 48: r = kernel_tex_image_interp(__tex_image_byte4_048, x, y); break;
		case 49: r = kernel_tex_image_interp(__tex_image_byte4_049, x, y); break;
		case 50: r = kernel_tex_image_interp(__tex_image_byte4_050, x, y); break;
		case 51: r = kernel_tex_image_interp(__tex_image_byte4_051, x, y); break;
		case 52: r = kernel_tex_image_interp(__tex_image_byte4_052, x, y); break;
		case 53: r = kernel_tex_image_interp(__tex_image_byte4_053, x, y); break;
		case 54: r = kernel_tex_image_interp(__tex_image_byte4_054, x, y); break;
		case 55: r = kernel_tex_image_interp(__tex_image_byte4_055, x, y); break;
		case 56: r = kernel_tex_image_interp(__tex_image_byte4_056, x, y); break;
		case 57: r = kernel_tex_image_interp(__tex_image_byte4_057, x, y); break;
		case 58: r = kernel_tex_image_interp(__tex_image_byte4_058, x, y); break;
		case 59: r = kernel_tex_image_interp(__tex_image_byte4_059, x, y); break;
		case 60: r = kernel_tex_image_interp(__tex_image_byte4_060, x, y); break;
		case 61: r = kernel_tex_image_interp(__tex_image_byte4_061, x, y); break;
		case 62: r = kernel_tex_image_interp(__tex_image_byte4_062, x, y); break;
		case 63: r = kernel_tex_image_interp(__tex_image_byte4_063, x, y); break;
		case 64: r = kernel_tex_image_interp(__tex_image_byte4_064, x, y); break;
		case 65: r = kernel_tex_image_interp(__tex_image_byte4_065, x, y); break;
		case 66: r = kernel_tex_image_interp(__tex_image_byte4_066, x, y); break;
		case 67: r = kernel_tex_image_interp(__tex_image_byte4_067, x, y); break;
		case 68: r = kernel_tex_image_interp(__tex_image_byte4_068, x, y); break;
		case 69: r = kernel_tex_image_interp(__tex_image_byte4_069, x, y); break;
		case 70: r = kernel_tex_image_interp(__tex_image_byte4_070, x, y); break;
		case 71: r = kernel_tex_image_interp(__tex_image_byte4_071, x, y); break;
		case 72: r = kernel_tex_image_interp(__tex_image_byte4_072, x, y); break;
		case 73: r = kernel_tex_image_interp(__tex_image_byte4_073, x, y); break;
		case 74: r = kernel_tex_image_interp(__tex_image_byte4_074, x, y); break;
		case 75: r = kernel_tex_image_interp(__tex_image_byte4_075, x, y); break;
		case 76: r = kernel_tex_image_interp(__tex_image_byte4_076, x, y); break;
		case 77: r = kernel_tex_image_interp(__tex_image_byte4_077, x, y); break;
		case 78: r = kernel_tex_image_interp(__tex_image_byte4_078, x, y); break;
		case 79: r = kernel_tex_image_interp(__tex_image_byte4_079, x, y); break;
		case 80: r = kernel_tex_image_interp(__tex_image_byte4_080, x, y); break;
		case 81: r = kernel_tex_image_interp(__tex_image_byte4_081, x, y); break;
		case 82: r = kernel_tex_image_interp(__tex_image_byte4_082, x, y); break;
		case 83: r = kernel_tex_image_interp(__tex_image_byte4_083, x, y); break;
		case 84: r = kernel_tex_image_interp(__tex_image_byte4_084, x, y); break;
		case 85: r = kernel_tex_image_interp(__tex_image_byte4_085, x, y); break;
		case 86: r = kernel_tex_image_interp(__tex_image_byte4_086, x, y); break;
		case 87: r = kernel_tex_image_interp(__tex_image_byte4_087, x, y); break;
		case 88: r = kernel_tex_image_interp(__tex_image_byte4_088, x, y); break;
		default:
			kernel_assert(0);
			return make_float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
#  else
	CUtexObject tex = kernel_tex_fetch(__bindless_mapping, id);
	/* float4, byte4 and half4 */
	if(kernel_tex_type(id) == IMAGE_DATA_TYPE_FLOAT4 || kernel_tex_type(id) == IMAGE_DATA_TYPE_BYTE4
	   || kernel_tex_type(id) == IMAGE_DATA_TYPE_HALF4 || kernel_tex_type(id) == IMAGE_DATA_TYPE_USHORT4)
		r = kernel_tex_image_interp_float4(tex, x, y);
	/* float, byte and half */
	else {
		float f = kernel_tex_image_interp_float(tex, x, y);
		r = make_float4(f, f, f, 1.0f);
	}
#  endif
#endif

#ifdef __KERNEL_SSE2__
	float alpha = r.w;

	if(use_alpha && alpha != 1.0f && alpha != 0.0f) {
		r_ssef = r_ssef / ssef(alpha);
		if(kernel_tex_type(id) == IMAGE_DATA_TYPE_BYTE4 || kernel_tex_type(id) == IMAGE_DATA_TYPE_BYTE)
			r_ssef = min(r_ssef, ssef(1.0f));
		r.w = alpha;
	}

	if(srgb) {
		r_ssef = color_srgb_to_scene_linear(r_ssef);
		r.w = alpha;
	}
#else
	if(use_alpha && r.w != 1.0f && r.w != 0.0f) {
		float invw = 1.0f/r.w;
		r.x *= invw;
		r.y *= invw;
		r.z *= invw;

		if(kernel_tex_type(id) == IMAGE_DATA_TYPE_BYTE4 || kernel_tex_type(id) == IMAGE_DATA_TYPE_BYTE) {
			r.x = min(r.x, 1.0f);
			r.y = min(r.y, 1.0f);
			r.z = min(r.z, 1.0f);
		}
	}

	if(srgb) {
		r.x = color_srgb_to_scene_linear(r.x);
		r.y = color_srgb_to_scene_linear(r.y);
		r.z = color_srgb_to_scene_linear(r.z);
	}
#endif

	return r;
}

/* Remap coordnate from 0..1 box to -1..-1 */
ccl_device_inline float3 texco_remap_square(float3 co)
{
	return (co - make_float3(0.5f, 0.5f, 0.5f)) * 2.0f;
}

ccl_device void svm_node_tex_image(KernelGlobals *kg, ShaderData *sd, int path_flag, float *stack, uint4 node)
{
	uint id = node.y;
	uint co_offset, out_offset, alpha_offset, srgb;
	uint projection, dx_offset, dy_offset;

	decode_node_uchar4(node.z, &co_offset, &out_offset, &alpha_offset, &srgb);
	decode_node_uchar4(node.w, &projection, &dx_offset, &dy_offset, NULL);

	float3 co = stack_load_float3(stack, co_offset);
	float2 tex_co;
	uint use_alpha = stack_valid(alpha_offset);
	if(projection == NODE_IMAGE_PROJ_SPHERE) {
		co = texco_remap_square(co);
		tex_co = map_to_sphere(co);
	}
	else if(projection == NODE_IMAGE_PROJ_TUBE) {
		co = texco_remap_square(co);
		tex_co = map_to_tube(co);
	}
	else {
		tex_co = make_float2(co.x, co.y);
	}
	
	differential ds, dt;
#ifdef __KERNEL_CPU__
	if(stack_valid(dx_offset) && stack_valid(dy_offset)) {
		float3 dx = stack_load_float3(stack, dx_offset);
		float3 dy = stack_load_float3(stack, dy_offset);
		float2 tex_co_dx, tex_co_dy;
		if(projection == NODE_IMAGE_PROJ_SPHERE) {
			dx = texco_remap_square(dx);
			tex_co_dx = map_to_sphere(dx);
			dy = texco_remap_square(dy);
			tex_co_dy = map_to_sphere(dy);
		}
		else if(projection == NODE_IMAGE_PROJ_TUBE) {
			dx = texco_remap_square(dx);
			tex_co_dx = map_to_tube(dx);
			dy = texco_remap_square(dy);
			tex_co_dy = map_to_tube(dy);
		}
		else {
			tex_co_dx = make_float2(dx.x, dx.y);
			tex_co_dy = make_float2(dy.x, dy.y);
		}
		ds.dx = tex_co_dx.x - tex_co.x;
		ds.dy = tex_co_dy.x - tex_co.x;
		dt.dx = tex_co_dx.y - tex_co.y;
		dt.dy = tex_co_dy.y - tex_co.y;
	}
	else
#endif
	{
		ds = differential_zero();
		dt = differential_zero();
	}
	float4 f = svm_image_texture(kg, id, tex_co.x, tex_co.y, ds, dt, srgb, use_alpha, path_flag);

	if(stack_valid(out_offset))
		stack_store_float3(stack, out_offset, make_float3(f.x, f.y, f.z));
	if(stack_valid(alpha_offset))
		stack_store_float(stack, alpha_offset, f.w);
}

ccl_device void svm_node_tex_image_box(KernelGlobals *kg, ShaderData *sd, float *stack, uint4 node, int *offset)
{
	/* get object space normal */
	float3 N = sd->N;

	N = sd->N;
	object_inverse_normal_transform(kg, sd, &N);

	/* project from direction vector to barycentric coordinates in triangles */
	N.x = fabsf(N.x);
	N.y = fabsf(N.y);
	N.z = fabsf(N.z);

	N /= (N.x + N.y + N.z);

	uint co_offset, out_offset, alpha_offset, srgb;
	uint dx_offset, dy_offset;
	decode_node_uchar4(node.z, &co_offset, &out_offset, &alpha_offset, &srgb);
	decode_node_uchar4(node.w, &dx_offset, &dy_offset, NULL, NULL);

	/* basic idea is to think of this as a triangle, each corner representing
	 * one of the 3 faces of the cube. in the corners we have single textures,
	 * in between we blend between two textures, and in the middle we a blend
	 * between three textures.
	 *
	 * the Nxyz values are the barycentric coordinates in an equilateral
	 * triangle, which in case of blending, in the middle has a smaller
	 * equilateral triangle where 3 textures blend. this divides things into
	 * 7 zones, with an if() test for each zone */

	float3 weight = make_float3(0.0f, 0.0f, 0.0f);
	float4 blend4 = read_node_float(kg, offset);
	float blend = blend4.x;
	float limit = 0.5f*(1.0f + blend);

	/* first test for corners with single texture */
	if(N.x > limit*(N.x + N.y) && N.x > limit*(N.x + N.z)) {
		weight.x = 1.0f;
	}
	else if(N.y > limit*(N.x + N.y) && N.y > limit*(N.y + N.z)) {
		weight.y = 1.0f;
	}
	else if(N.z > limit*(N.x + N.z) && N.z > limit*(N.y + N.z)) {
		weight.z = 1.0f;
	}
	else if(blend > 0.0f) {
		/* in case of blending, test for mixes between two textures */
		if(N.z < (1.0f - limit)*(N.y + N.x)) {
			weight.x = N.x/(N.x + N.y);
			weight.x = saturate((weight.x - 0.5f*(1.0f - blend))/blend);
			weight.y = 1.0f - weight.x;
		}
		else if(N.x < (1.0f - limit)*(N.y + N.z)) {
			weight.y = N.y/(N.y + N.z);
			weight.y = saturate((weight.y - 0.5f*(1.0f - blend))/blend);
			weight.z = 1.0f - weight.y;
		}
		else if(N.y < (1.0f - limit)*(N.x + N.z)) {
			weight.x = N.x/(N.x + N.z);
			weight.x = saturate((weight.x - 0.5f*(1.0f - blend))/blend);
			weight.z = 1.0f - weight.x;
		}
		else {
			/* last case, we have a mix between three */
			weight.x = ((2.0f - limit)*N.x + (limit - 1.0f))/(2.0f*limit - 1.0f);
			weight.y = ((2.0f - limit)*N.y + (limit - 1.0f))/(2.0f*limit - 1.0f);
			weight.z = ((2.0f - limit)*N.z + (limit - 1.0f))/(2.0f*limit - 1.0f);
		}
	}
	else {
		/* Desperate mode, no valid choice anyway, fallback to one side.*/
		weight.x = 1.0f;
	}

	/* now fetch textures */
	float3 co = stack_load_float3(stack, co_offset);
	uint id = node.y;

	float4 f = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
	uint use_alpha = stack_valid(alpha_offset);

	differential ds = differential_zero();
	differential dt = differential_zero();

#ifdef __KERNEL_CPU__
	float3 offset_x = make_float3(0.0f, 0.0f, 0.0f);
	float3 offset_y = make_float3(0.0f, 0.0f, 0.0f);
	if(stack_valid(dx_offset) && stack_valid(dy_offset)) {
		offset_x = stack_load_float3(stack, dx_offset);
		offset_y = stack_load_float3(stack, dy_offset);
	}
#endif

	if(weight.x > 0.0f) {
#ifdef __KERNEL_CPU__
		ds.dx = co.y - offset_x.y;
		ds.dy = co.y - offset_y.y;
		dt.dx = co.z - offset_x.z;
		dt.dy = co.z - offset_y.z;
#endif
		f += weight.x*svm_image_texture(kg, id, co.y, co.z, ds, dt, srgb, use_alpha, false);
	}
	if(weight.y > 0.0f) {
#ifdef __KERNEL_CPU__
		ds.dx = co.x - offset_x.x;
		ds.dy = co.x - offset_y.x;
		dt.dx = co.z - offset_x.z;
		dt.dy = co.z - offset_y.z;
#endif
		f += weight.y*svm_image_texture(kg, id, co.x, co.z, ds, dt, srgb, use_alpha, false);
	}
	if(weight.z > 0.0f) {
#ifdef __KERNEL_CPU__
		ds.dx = co.y - offset_x.y;
		ds.dy = co.y - offset_y.y;
		dt.dx = co.x - offset_x.x;
		dt.dy = co.x - offset_y.x;
#endif
		f += weight.z*svm_image_texture(kg, id, co.y, co.x, ds, dt, srgb, use_alpha, false);
	}

	if(stack_valid(out_offset))
		stack_store_float3(stack, out_offset, make_float3(f.x, f.y, f.z));
	if(stack_valid(alpha_offset))
		stack_store_float(stack, alpha_offset, f.w);
}

ccl_device void svm_node_tex_environment(KernelGlobals *kg, ShaderData *sd, int path_flag, float *stack, uint4 node)
{
	uint id = node.y;
	uint co_offset, out_offset, alpha_offset, srgb;
	uint projection, dx_offset, dy_offset;

	decode_node_uchar4(node.z, &co_offset, &out_offset, &alpha_offset, &srgb);
	decode_node_uchar4(node.w, &projection, &dx_offset, &dy_offset, NULL);

	uint use_alpha = stack_valid(alpha_offset);
	float4 f;

	float3 co = stack_load_float3(stack, co_offset);
	float2 uv;

	co = normalize(co);
#ifdef __OIIO__
	float3 dRdx, dRdy;
	if(stack_valid(dx_offset) && stack_valid(dy_offset)) {
		dRdx = co - normalize(stack_load_float3(stack, dx_offset));
		dRdy = co - normalize(stack_load_float3(stack, dy_offset));
	}
	else
	{
		dRdx = make_float3(0.0f, 0.0f, 0.0f);
		dRdy = make_float3(0.0f, 0.0f, 0.0f);
	}
	if(kg->oiio && kg->oiio->textures.size() > id && kg->oiio->textures[id].handle && projection == 0) {
		OIIO::TextureOpt options;
		options.swrap = options.twrap = kg->oiio->textures[id].extension;
		options.anisotropic = 8;

		if(path_flag & NEAREST_LOOKUP_PATHS && !(path_flag & PATH_RAY_SINGULAR)) {
			options.interpmode = OIIO::TextureOpt::InterpClosest;
			options.mipmode = OIIO::TextureOpt::MipModeOneLevel;
		}
		else {
			options.interpmode = kg->oiio->textures[id].interpolation;
			options.mipmode = OIIO::TextureOpt::MipModeTrilinear;
		}
		
		if(path_flag & DIFFUSE_BLUR_PATHS) {
			options.sblur = options.tblur = kg->oiio->diffuse_blur;
		} else if(path_flag & PATH_RAY_GLOSSY) {
			options.sblur = options.tblur = kg->oiio->glossy_blur;
		}
		else {
			options.sblur = options.tblur = 0.0f;
		}
		
		bool success = kg->oiio->tex_sys->environment (kg->oiio->textures[id].handle, (OIIO::TextureSystem::Perthread*)kg->oiio_tdata, options,
													   Imath::V3f(co.x, -co.y, co.z), Imath::V3f(dRdx.x, -dRdx.y, dRdx.z),
													   Imath::V3f(dRdy.x, -dRdy.y, dRdy.z), use_alpha ? 4 : 3, (float*)&f);

		if(!success) {
			(void) kg->oiio->tex_sys->geterror();
		}
	}
	else {
#endif

	if(projection == 0)
		uv = direction_to_equirectangular(co);
	else
		uv = direction_to_mirrorball(co);

	f = svm_image_texture(kg, id, uv.x, uv.y, differential_zero(), differential_zero(), srgb, use_alpha, path_flag);
#ifdef __OIIO__
	}
#endif

	if(stack_valid(out_offset))
		stack_store_float3(stack, out_offset, make_float3(f.x, f.y, f.z));
	if(stack_valid(alpha_offset))
		stack_store_float(stack, alpha_offset, f.w);
}

ccl_device float minimum_distance(float2 v, float2 w, float2 p) {
    // Return minimum distance between line segment vw and point p
    float2 diff = v - w;
    float l2 = diff.x * diff.x + diff.y * diff.y;  // i.e. |w-v|^2 -  avoid a sqrt

    if (l2 == 0.0f)
        return len(p - v);   // v == w case

    // Consider the line extending the segment, parameterized as v + t (w - v).
    // We find projection of point p onto the line. 
    // It falls where t = [(p-v) . (w-v)] / |w-v|^2
    // We clamp t from [0,1] to handle points outside the segment vw.
    float t = max(0.0F, min(1.0F, dot(p-v, w-v) / l2));
    float2 projection = v + t * (w - v);  // Projection falls on the segment

    return len(p - projection);
}

ccl_device void svm_node_tex_curve(KernelGlobals *kg, ShaderData *sd, float *stack, uint4 node, int *offset)
{
    uint co_offset, fill_in_offset, background_in_offset, out_offset;
    uint curve_thickness_offset, curve_location_offset, curve_scale_offset;
    uint curve_type;

    decode_node_uchar4(node.z, &co_offset, &fill_in_offset, &background_in_offset, &out_offset);
    decode_node_uchar4(node.w, &curve_thickness_offset, &curve_location_offset, &curve_scale_offset, &curve_type);
    uint slot = node.y;

    uint4 node2 = read_node(kg, offset);
    int width = node2.x;


    float4 f;
    if (slot == (uint) -1 || width <= 0) {
        float3 background_color = stack_load_float3(stack, background_in_offset);
        f = make_float4(background_color.x, background_color.y, background_color.z, 1.0);
    } else {

        float3 co = stack_load_float3(stack, co_offset);
        float3 fill_color = stack_load_float3(stack, fill_in_offset);
        float3 background_color = stack_load_float3(stack, background_in_offset);
        float curve_thickness = stack_load_float(stack, curve_thickness_offset);
        float3 curve_location = stack_load_float3(stack, curve_location_offset);
        float3 curve_scale = stack_load_float3(stack, curve_scale_offset);
        
        // Binary search line segments
        int min_xi = 0;
        int max_xi = width-1;
        int min_xr = 0;
        int max_xr = width-1;
        
        float cox = (co.x - curve_location.x) / curve_scale.x;
        
        while ( (max_xi - min_xi) > 1) {
            int c_xi = (min_xi + max_xi) / 2;
            float4 c_ex = svm_image_texture(kg, slot, (float)c_xi/(width-1), 0.75F, differential_zero(), differential_zero(), false, false, 0);

            // Narrow the range
            if (cox <= c_ex.x) {
                max_xi = c_xi;
                max_xr = __float_as_int(c_ex.z);
            } else {
                min_xi = c_xi;
                min_xr = __float_as_int(c_ex.y);
            }
            
            // Are we done?
            if (min_xi >= c_xi && max_xi <= (c_xi+1))
                break;
        }
        
        kernel_assert(min_xr >= 0 && min_xr < width);
        kernel_assert(max_xr >= 0 && max_xr < width);
        
        // Render
        float grad = 1.0f;

        for (int t = min_xr; t <= max_xr; ++t) {
            float4 ls0 = svm_image_texture(kg, slot, (float)t/(width-1), 0.25F, differential_zero(), differential_zero(), false, false, 0);

            float2 p0,p1,co2;
            p0.x = ls0.x * curve_scale.x + curve_location.x;
            p0.y = ls0.y * curve_scale.y + curve_location.y;
            p1.x = ls0.z * curve_scale.x + curve_location.x;
            p1.y = ls0.w * curve_scale.y + curve_location.y;
            co2.x = co.x;
            co2.y = co.y;

            // Line
            if (curve_type == 0) {
                if (minimum_distance(p0, p1, co2) < curve_thickness) {

                    grad = 0.0f;
                    break;
                }

            // Fill
            } else if (curve_type == 1) {
                kernel_assert(p0.x <= p1.x);

                if (co2.x < p0.x || co2.x >= p1.x)
                    continue;

                float tx = (co2.x - p0.x) / (p1.x - p0.x);
                float y = p0.y + (p1.y - p0.y) * tx;

                if (y < co2.y) {
                    grad = !grad;
                }

            // Grad
            } else {

                float d = minimum_distance(p0, p1, co2);
                if (d < curve_thickness) {
                    grad = min(grad, d/curve_thickness);
                }

            }

        }
        
        f = make_float4((1.0f-grad) * fill_color.x + grad * background_color.x,
                        (1.0f-grad) * fill_color.y + grad * background_color.y,
                        (1.0f-grad) * fill_color.z + grad * background_color.z,
                        1.0f);

    }

    if(stack_valid(out_offset))
        stack_store_float3(stack, out_offset, make_float3(f.x, f.y, f.z));
}

CCL_NAMESPACE_END

