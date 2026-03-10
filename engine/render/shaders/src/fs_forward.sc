$input v_worldPos, v_normal, v_color0

// Freak Engine - Forward fragment shader
// Single directional light + ambient + emissive + distance fog.
// This is the atmosphere foundation for Freakland.

#include <bgfx_shader.sh>

// Material uniforms (set per-draw)
uniform vec4 u_materialDiffuse;   // xyz = diffuse color, w = roughness
uniform vec4 u_materialEmissive;  // xyz = emissive color, w = unused

// Lighting uniforms (set once per frame)
uniform vec4 u_lightDir;       // xyz = normalized direction TO light, w = intensity
uniform vec4 u_lightColor;     // xyz = light color, w = unused
uniform vec4 u_ambientColor;   // xyz = ambient color, w = unused

// Fog uniforms (set once per frame)
uniform vec4 u_fogColor;       // xyz = fog color, w = unused
uniform vec4 u_fogParams;      // x = density, y = start, z = end, w = unused

// Camera position for fog distance calculation
uniform vec4 u_cameraPos;      // xyz = camera world position, w = unused

void main()
{
    vec3 normal = normalize(v_normal);

    // ── Directional light (half-Lambert for softer shadows) ──
    float NdotL = dot(normal, u_lightDir.xyz);
    float halfLambert = NdotL * 0.5 + 0.5;
    halfLambert = halfLambert * halfLambert; // Squared for contrast

    vec3 diffuse = u_materialDiffuse.xyz * v_color0.xyz;
    vec3 lit = diffuse * (u_lightColor.xyz * halfLambert * u_lightDir.w + u_ambientColor.xyz);

    // ── Emissive (additive, unaffected by lighting) ──
    lit += u_materialEmissive.xyz;

    // ── Distance fog ──
    float dist = length(v_worldPos - u_cameraPos.xyz);
    // Linear fog with smooth falloff
    float fogFactor = clamp((dist - u_fogParams.y) / (u_fogParams.z - u_fogParams.y), 0.0, 1.0);
    // Apply density for artistic control
    fogFactor = 1.0 - exp(-fogFactor * fogFactor * u_fogParams.x * 10.0);

    vec3 finalColor = mix(lit, u_fogColor.xyz, fogFactor);

    gl_FragColor = vec4(finalColor, 1.0);
}
