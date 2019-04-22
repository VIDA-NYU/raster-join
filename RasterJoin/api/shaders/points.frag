#version 410

flat in float val;
out vec4 fragColor;

void main()
{
    fragColor = vec4(1,val,0,1);
}
