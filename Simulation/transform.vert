
#version 450
#extension GL_ARB_shader_draw_parameters : require

layout(location = 0) in vec3 vert;
layout(location = 2) in vec2 tex_coords;
layout(location = 1) in vec3 normal;

out vec4 gl_Position;
out vec3 frag_position;
out float frag_depth;
out vec2 frag_texcoords;
out vec3 frag_normal;
flat out int drawId;

uniform mat4 projection;
uniform mat4 view_model;
uniform mat4 trans_inverse_view_model;
uniform float near = 1.0f;
uniform float far = 1000.0f;

void main() {
	vec4 eye_v = view_model * vec4(vert, 1);

	frag_position = eye_v.xyz;
	frag_texcoords = tex_coords;
	frag_normal = (trans_inverse_view_model * vec4(normal, 1)).xyz;
	drawId = gl_DrawIDARB;

	gl_Position = projection * eye_v;
	
	frag_depth = (-eye_v.z - near) / (far - near);
}
