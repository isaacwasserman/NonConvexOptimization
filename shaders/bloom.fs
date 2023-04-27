#version 400

in vec2 uv;

uniform sampler2D view;
uniform vec2 resolution;

out vec4 FragColor;

void main()
{   
    FragColor = texture(view, uv);
    if(length(FragColor.xyz) == 0){
        FragColor = vec4(0.9, 0.95, 1, 1);
    }
    
    // float two_Pi = 6.28318530718;
    // float blur_directions = 16.0;
    // float blur_quality = 3.0;
    // float blur_size = 5.0;

    // vec2 Radius = blur_size / resolution.xy;
    
    // vec2 uv = gl_FragCoord.xy / resolution.xy;
    // vec4 blur_color = texture(view, uv);
    
    // int pixels_included = 0;
    // for(float d = 0.0; d < two_Pi; d += two_Pi / blur_directions) {
    //     for(float i = 1.0 / blur_quality; i <= 1.0; i += 1.0 / blur_quality) {
    //         vec4 additional_color = texture(view, uv + vec2(cos(d), sin(d)) * Radius * i);
    //         if(length(additional_color) > bloom_threshold) {
    //             pixels_included++;
    //             blur_color += additional_color;
    //         }
    //     }
    // }
    
    // blur_color /= pixels_included + 1;
    // FragColor = blur_color;
    

    // int red_offset = 0;
    // int green_offset = 10;
    // int blue_offset = 20;
    // FragColor.r = texture(view, uv + vec2(red_offset / resolution.x, red_offset / resolution.y)).r;
    // FragColor.g = texture(view, uv + vec2(green_offset / resolution.x, green_offset / resolution.y)).g;
    // FragColor.b = texture(view, uv + vec2(blue_offset / resolution.x, blue_offset / resolution.y)).b;
}