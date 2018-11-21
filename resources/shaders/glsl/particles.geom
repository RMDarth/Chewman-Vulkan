#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

layout(location = 0) in vec3 geomColor[];

layout(set = 1, binding = 0) uniform UBO
{
    mat4 projection;
} ubo;

layout( location = 0 ) out vec2 fragTexCoord;
layout( location = 1 ) out vec3 fragColor;

const float SIZE = 0.05;

void main()
{
  vec4 position = gl_in[0].gl_Position;

  gl_Position = ubo.projection * (gl_in[0].gl_Position + vec4( -SIZE, SIZE, 0.0, 0.0 ));
  fragTexCoord = vec2( -1.0, 1.0 );
  fragColor = geomColor[0];
  EmitVertex();

  gl_Position = ubo.projection * (gl_in[0].gl_Position + vec4( -SIZE, -SIZE, 0.0, 0.0 ));
  fragTexCoord = vec2( -1.0, -1.0 );
  fragColor = geomColor[0];
  EmitVertex();

  gl_Position = ubo.projection * (gl_in[0].gl_Position + vec4( SIZE, SIZE, 0.0, 0.0 ));
  fragTexCoord = vec2( 1.0, 1.0 );
  fragColor = geomColor[0];
  EmitVertex();

  gl_Position = ubo.projection * (gl_in[0].gl_Position + vec4( SIZE, -SIZE, 0.0, 0.0 ));
  fragTexCoord = vec2( 1.0, -1.0 );
  fragColor = geomColor[0];
  EmitVertex();

  EndPrimitive();
}
