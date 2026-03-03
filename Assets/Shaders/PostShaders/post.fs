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
float gamma = 0.8;
float numColors = 10.0;

// Custom uniforms
uniform vec2 u_frameSize;
uniform float offset = 0.0;
uniform float brightness = 1.3;
uniform bool shake;

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

vec4 applyVignette(vec4 color, vec2 uv, float miny, float maxy)
{
    vec2 position = ((uv) - vec2(0.5));           
    float dist = abs(position.y)/(maxy - miny);

    float ratio = 0.49;
    float strength = 4;
    
    float radius = ratio * strength;
    float softness = (1.0 - ratio) * strength;

    float vignette = smoothstep(radius, radius - softness, dist);

    color.rgb = color.rgb - (1.0 - vignette);

    return color;
}

vec4 applyScanline(vec4 color, vec2 uv)
{
    float frequency = u_frameSize.y/10.0;
    // Scanlines method 2
    float globalPos = (uv.y + offset) * frequency;
    float wavePos = cos((fract(globalPos) - 0.5)* 1.1);

    return mix(vec4(0.1, 0.0, 0.4, 1), color, wavePos);

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

vec2 curve(vec2 uv)
{
	uv = (uv - 0.5) * 2.0;
	uv *= 1.1;
	uv.x *= 1.0 + pow((abs(uv.y) / 5.0), 2.0);
	uv.y *= 1.0 + pow((abs(uv.x) / 4.0), 2.0);
	uv = (uv / 2.0) + 0.5;
	uv =  uv *0.92 + 0.04;

	return uv;
}

vec2 fisheyeUV()
{
    const float PI = 3.1415926535897932384626433832795;
    float aperture = 180.0;
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
    float pixelWidth = 5;
    float pixelHeight = 5;
    float renderWidth = u_frameSize.x;
	float renderHeight = u_frameSize.y;

    float dx = pixelWidth*(1.0/(1 * renderWidth));
    float dy = pixelHeight*(1.0/(1 * renderHeight));

    vec2 coord = vec2(dx*floor(uv.x/dx), dy*floor(uv.y/dy));

    vec3 tc = texture(texture0, coord).rgb;

    return vec4(tc, 1.0);
}

vec4 applyBlur(vec4 color, vec2 uv)
{
    float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
    float weight[3] = float[](0.2, 0.2, 0.2);

        // Texel color fetching from texture sampler
    vec3 texelColor = color.rgb*weight[0];

    for (int i = 1; i < 3; i++)
    {
        texelColor += texture(texture0, uv + vec2(offset[i])/u_frameSize.x, 0.0).rgb*weight[i];
        texelColor += texture(texture0, uv - vec2(offset[i])/u_frameSize.y, 0.0).rgb*weight[i];
    }

    return vec4(texelColor, 1.0);
}

void main()
{
    // vec2 uv = fragTexCoord;
    float dim = min(u_frameSize.x, u_frameSize.y);
    float minx = (u_frameSize.x - u_frameSize.y) / (2.0 * u_frameSize.x);
    float maxx = (u_frameSize.x + u_frameSize.y) / (2.0 * u_frameSize.x);
    float miny = (u_frameSize.y - u_frameSize.x) / (2.0 * u_frameSize.y);
    float maxy = (u_frameSize.y + u_frameSize.x) / (2.0 * u_frameSize.y);

    if (u_frameSize.x > u_frameSize.y)
    {
        miny = 0.0;
        maxy = 1.0;
    }
    else
    {
        minx = 0.0;
        maxx = 1.0;
    }


    vec2 uv = fragTexCoord;
    uv = curve(uv);
    vec4 color = texture(texture0, uv);
    if (shake)
    {
        color = applyBlur(color, uv);
    }
    
    //vec4 vfx = PostFX(texture0, uv);
    // Texel color fetching from texture sampler
    // color = vfx;
    // color = applyPixelizer(uv);
    color = applyPosterization(color);
    color = applyBloom(color, uv);
    color = applyScanline(color, uv);
    color = applyVignette(color, uv, miny, maxy);
    // color = mix(color, vfx, 1);  
    color.rgb = pow(color.rgb, vec3(1.0/brightness));
 
    // clipped unwanted uvs only render square
    if (uv.x < minx || uv.x > maxx)
        color *= 0.0;
    if (uv.y < miny || uv.y > maxy)
        color *= 0.0;

    finalColor = color; 
}