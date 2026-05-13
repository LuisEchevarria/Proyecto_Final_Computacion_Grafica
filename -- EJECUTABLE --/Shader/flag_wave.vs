#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// --- Parametros de onda ---
uniform float time;       // glfwGetTime() enviado desde la app
uniform float waveAmp;    // Amplitud A
uniform float waveK;      // Frecuencia espacial k
uniform float waveOmega;  // Velocidad angular omega

// --- Geometria de la lona ---
uniform float flagXMin;   // x del borde fijo (mastil)
uniform float flagWidth;  // Ancho total: free_edge_x - mastil_x
uniform float flagZ;      // z del plano de la lona (para aislar sus vertices)

void main()
{
    vec3 pos = position;

    // --- Identificar vertices de la lona ---
    // La lona es una malla plana a z = flagZ.
    // El mastil, la base y el cap son objetos 3D con z variable: los excluimos.
    float isFlagVert = 1.0 - step(0.015, abs(pos.z - flagZ));

    // --- Peso de anclaje: 0 en el mastil (fijo), 1 en el borde libre ---
    float weight = isFlagVert * clamp((pos.x - flagXMin) / flagWidth, 0.0, 1.0);

    // --- Ecuacion de onda: Z(x,t) = A * sin(k*x - omega*t) * weight(x) ---
    float disp = waveAmp * sin(waveK * pos.x - waveOmega * time);
    pos.z += disp * weight;

    // --- Recalculo de normal por derivada parcial (Phong Shading fisico) ---
    // dZ/dx = A * k * cos(k*x - omega*t) * weight
    float dZdx = waveAmp * waveK * cos(waveK * pos.x - waveOmega * time) * weight;
    // Tangente en X: (1, 0, dZdx)  |  Tangente en Y: (0, 1, 0)
    // Normal = cross((1,0,dZdx), (0,1,0)) = (-dZdx, 0, 1) normalizado
    vec3 waveNormal = normalize(vec3(-dZdx, 0.0, 1.0));

    // weight = 0 -> normal original (mastil/base/cap sin cambios)
    // weight > 0 -> normal de onda calculada (lona en movimiento)
    vec3 finalNormal = mix(normal, waveNormal, weight);

    gl_Position = projection * view * model * vec4(pos, 1.0f);
    FragPos     = vec3(model * vec4(pos, 1.0f));
    Normal      = mat3(transpose(inverse(model))) * finalNormal;
    TexCoords   = texCoords;
}
