#version 330


// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add your custom variables here
uniform vec2 u_frameSize;

vec4 applyVignette(vec4 color)
{
    vec2 position = (gl_FragCoord.xy / u_frameSize) - vec2(0.5);           
    float dist = length(position);

    float radius = 1.4;
    float softness = 1;
    float vignette = smoothstep(radius, radius - softness, dist);

    color.rgb = color.rgb - (1.0 - vignette);

    return color;
}

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);

    // NOTE: Implement here your fragment shader code

    finalColor = applyVignette(texelColor);
}
