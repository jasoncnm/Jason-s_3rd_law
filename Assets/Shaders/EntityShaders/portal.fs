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
uniform float time;

float random( float seed )
{
	return fract( 543.2543 * sin( dot( vec2( seed, seed ), vec2( 3525.46, -54.3415 ) ) ) );
}

vec4 glitch()
{

    // 振動の強さ
    float shake_power = 0.005;

    // hint_range( 0.0, 1.0 )
    float shake_rate = 1;

    // 振動速度
    float shake_speed = 5.0;

    // 振動ブロックサイズ
    float shake_block_size = 16;

    // 色の分離率
    // hint_range( 0.0, 1.0 ) 
    float shake_color_rate = 1.0;

    float enable_shift = float(random( trunc( time * shake_speed ) ) < shake_rate);

    vec2 fixed_uv = fragTexCoord;

    fixed_uv.x += (
        random(
            ( trunc( fragTexCoord.y * shake_block_size ) / shake_block_size )
        +	time
        ) - 0.5
    ) * shake_power * enable_shift;

    vec4 pixel_color = textureLod( texture0, fixed_uv, 0.0 );

    pixel_color.r = mix(
        pixel_color.r
    ,	textureLod( texture0, fixed_uv + vec2( shake_color_rate, 0.0 ), 0.0 ).r
    ,	enable_shift
    );
    pixel_color.b = mix(
        pixel_color.b
    ,	textureLod( texture0, fixed_uv + vec2( -shake_color_rate, 0.0 ), 0.0 ).b
    ,	enable_shift
    );

    return pixel_color;
}

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = glitch();
    // NOTE: Implement here your fragment shader code

    // final color is the color from the texture 
    //    times the tint color (colDiffuse)
    //    times the fragment color (interpolated vertex color)
    finalColor = texelColor*colDiffuse*fragColor;
}

