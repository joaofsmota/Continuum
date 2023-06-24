//
#version 460 core

// extents of grid in world coordinates
float grid_size = 10.0;

// size of one cell
float grid_cell_size = 0.0025;

// color of thin lines
vec4 grid_color_thin = vec4(0.5, 0.5, 0.5, 1.0);

// color of thick lines (every tenth line)
vec4 grid_color_thick = vec4(0.0, 0.0, 0.0, 1.0);

// minimum number of pixels between cell lines before LOD switch should occur. 
const float grid_min_pixel_between_cells = 2.0;

const vec3 pos[4] = vec3[4](
	vec3(-1.0, -1.0, 0.0),
	vec3( 1.0, -1.0, 0.0),
	vec3( 1.0, 1.0,  0.0),
	vec3(-1.0, 1.0,  0.0)
);

const int indices[6] = int[6](
	0, 1, 2, 2, 3, 0
);

float log10(float x)
{
	return log(x) / log(10.0);
}

float satf(float x)
{
	return clamp(x, 0.0, 1.0);
}

vec2 satv(vec2 x)
{
	return clamp(x, vec2(0.0), vec2(1.0));
}

float max2(vec2 v)
{
	return max(v.x, v.y);
}

vec4 grid_color(vec2 uv, vec2 cam_pos)
{
	vec2 dudv = vec2(
		length(vec2(dFdx(uv.x), dFdy(uv.x))),
		length(vec2(dFdx(uv.y), dFdy(uv.y)))
	);

	float lodLevel = max(0.0, log10((length(dudv) * grid_min_pixel_between_cells) / grid_cell_size) + 1.0);
	float lodFade = fract(lodLevel);

	// cell sizes for lod0, lod1 and lod2
	float lod0 = grid_cell_size * pow(10.0, floor(lodLevel));
	float lod1 = lod0 * 10.0;
	float lod2 = lod1 * 10.0;

	// each anti-aliased line covers up to 4 pixels
	dudv *= 4.0;

	// calculate absolute distances to cell line centers for each lod and pick max X/Y to get coverage alpha value
	float lod0a = max2( vec2(1.0) - abs(satv(mod(uv, lod0) / dudv) * 2.0 - vec2(1.0)) );
	float lod1a = max2( vec2(1.0) - abs(satv(mod(uv, lod1) / dudv) * 2.0 - vec2(1.0)) );
	float lod2a = max2( vec2(1.0) - abs(satv(mod(uv, lod2) / dudv) * 2.0 - vec2(1.0)) );

	uv -= cam_pos;

	// blend between falloff colors to handle LOD transition
	vec4 c = lod2a > 0.0 ? grid_color_thick : lod1a > 0.0 ? mix(grid_color_thick, grid_color_thin, lodFade) : grid_color_thin;

	// calculate opacity falloff based on distance to grid extents
	float opacityFalloff = (1.0 - satf(length(uv) / grid_size));

	// blend between LOD level alphas and scale with opacity falloff
	c.a *= (lod2a > 0.0 ? lod2a : lod1a > 0.0 ? lod1a : (lod0a * (1.0-lodFade))) * opacityFalloff;

	return c;
}

layout (location=0) in vec2 uv;
layout (location=1) in vec2 cam_pos;
layout (location=0) out vec4 out_FragColor;

void main()
{
	out_FragColor = grid_color(uv, cam_pos);
};