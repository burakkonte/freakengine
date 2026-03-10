$input a_position, a_normal, a_color0
$output v_worldPos, v_normal, v_color0

// Freak Engine - Forward vertex shader
// Transforms vertices to clip space, passes world position
// and normal to fragment shader for lighting and fog.

#include <bgfx_shader.sh>

void main()
{
    // Transform to world space
    vec3 worldPos = mul(u_model[0], vec4(a_position, 1.0)).xyz;
    v_worldPos = worldPos;

    // Transform normal to world space (using upper-left 3x3 of model matrix)
    // This is correct for uniform scale. Non-uniform scale would need
    // the inverse transpose, but we're not doing that in M1.
    v_normal = normalize(mul(u_model[0], vec4(a_normal, 0.0)).xyz);

    // Pass vertex color through
    v_color0 = a_color0;

    // Final clip-space position
    gl_Position = mul(u_viewProj, vec4(worldPos, 1.0));
}
