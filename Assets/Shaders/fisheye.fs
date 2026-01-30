#version 330

in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

// NOTE: Add your custom variables here

const float PI = 3.1415926535;

void main()
{
    float aperture = 178.0;
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

    fragColor = texture(texture0, uv);

}