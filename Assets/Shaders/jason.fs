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
const float samples = 2;          // Pixels per axis; higher = bigger glow, worse performance
const float quality = 6;          // Defines size factor: Lower = smaller glow, better quality
float stitchingSize = 5.0;
float gamma = 0.8;
float numColors = 10.0;


uniform int invert = 0;
uniform vec2 u_frameSize;
uniform float offset = 0.0;
uniform float brightness = 1.2;

vec4 PostFX(sampler2D tex, vec2 uv)
{
	float renderWidth = u_frameSize.x;
	float renderHeight = u_frameSize.y;

    vec4 c = vec4(0.0);
    float size = stitchingSize;
    vec2 cPos = uv*vec2(renderWidth, renderHeight);
    vec2 tlPos = floor(cPos/vec2(size, size));
    tlPos *= size;

    int remX = int(mod(cPos.x, size));
    int remY = int(mod(cPos.y, size));

    if (remX == 0 && remY == 0) tlPos = cPos;

    vec2 blPos = tlPos;
    blPos.y += (size - 1.0);

    if ((remX == remY) || (((int(cPos.x) - int(blPos.x)) == (int(blPos.y) - int(cPos.y)))))
    {
        if (invert == 1) c = vec4(0.2, 0.15, 0.05, 1.0);
        else c = texture(tex, tlPos*vec2(1.0/renderWidth, 1.0/renderHeight))*1.4;
    }
    else
    {
        if (invert == 1) c = texture(tex, tlPos*vec2(1.0/renderWidth, 1.0/renderHeight))*1.4;
        else c = vec4(0.0, 0.0, 0.0, 1.0);
    }

    return c;
}

vec4 applyBloom(vec4 color, vec2 uv)
{
    vec4 sum = vec4(0);
    vec2 sizeFactor = vec2(1)/u_frameSize*quality;

    // Texel color fetching from texture sampler

    const int range = int(samples - 1) / 2;

    for (int x = -range; x <= range; x++)
    {
        for (int y = -range; y <= range; y++)
        {
            sum += texture(texture0, uv + vec2(x, y)*sizeFactor);
        }
    }
    return ((sum/(samples*samples)) + color)*colDiffuse;
}

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

vec4 applyScanline(vec4 color, vec2 uv)
{
    float frequency = u_frameSize.y/3.0;
    // Scanlines method 2
    float globalPos = (uv.y + offset) * frequency;
    float wavePos = cos((fract(globalPos) - 0.5)*1.5);

    return mix(vec4(0.2, 0.0, 0.5, 1), color, wavePos);

}

vec4 applyPosterization(vec4 color)
{
    vec3 result = color.rgb;
    result = pow(result, vec3(gamma, gamma, gamma));
    result = result*numColors;
    result = floor(result);
    result = result/numColors;
    result = pow(result, vec3(1.0/gamma));

    return vec4(result, 1.0);
}

vec2 fisheyeUV()
{
    const float PI = 2;
    float aperture = 95.0;
    float apertureHalf = 0.5*aperture*(PI/180.0);
    float maxFactor = sin(apertureHalf);

    vec2 uv = vec2(0);
    vec2 xy = 2.0*fragTexCoord.xy - 1.0;
    float d = length(xy);


//    if (d < (2.0 - maxFactor))
    if (d > 0)
    {
        d = length(xy*maxFactor);
        float z = sqrt(1.0 - d*d);
        float r = atan(d, z)/PI;
        float phi = atan(xy.y, xy.x);

        uv.x = r*cos(phi) + 0.5;
        uv.y = r*sin(phi) + 0.5;
    }
    else
    {
        uv = fragTexCoord.xy;
        // fragColor = vec4(vec3(0), 1);
    }

    return uv;
}

vec4 applyPixelizer(vec2 uv)
{
    float pixelWidth = 2;
    float pixelHeight = 2;
    float renderWidth = u_frameSize.x;
	float renderHeight = u_frameSize.y;

    float dx = pixelWidth*(1.0/(1 * renderWidth));
    float dy = pixelHeight*(1.0/(1 * renderHeight));

    vec2 coord = vec2(dx*floor(uv.x/dx), dy*floor(uv.y/dy));

    vec3 tc = texture(texture0, coord).rgb;

    return vec4(tc, 1.0);
}

void main()
{
    // vec2 uv = fragTexCoord;
    vec2 uv = fisheyeUV();
    // vec4 vfx = PostFX(texture0, uv);
    // Texel color fetching from texture sampler
    vec4 color = texture(texture0, uv);
    // color = vfx;
    color = applyPixelizer(uv);
    color = applyPosterization(color);
    color = applyVignette(color);
    color = applyBloom(color, uv);
    color = applyScanline(color, uv);
    // color = mix(color, vfx, 1);
    
    color.rgb = pow(color.rgb, vec3(1.0/brightness));
    // NOTE: Implement here your fragment shader code
    finalColor = color; 
}