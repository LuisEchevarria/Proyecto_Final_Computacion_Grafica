#version 330 core
in vec3 TexCoords;
out vec4 color;

// Recibimos AMBOS cubemaps
uniform samplerCube skyboxDay;
uniform samplerCube skyboxNight;

// Recibimos el factor de mezcla que calculamos en C++ (0.0 a 1.0)
uniform float blendFactor; 

void main()
{
    // Extraemos el color de ambas texturas en la misma coordenada
    vec4 dayColor = texture(skyboxDay, TexCoords);
    vec4 nightColor = texture(skyboxNight, TexCoords);
    
    // mix() hace el fundido: si blendFactor es 0.0 da noche, si es 1.0 da día
    color = mix(nightColor, dayColor, blendFactor);
}