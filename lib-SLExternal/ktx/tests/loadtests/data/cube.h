
static const float cube_face[] =
{
	-10.0f, +10.0f, +10.0f, /* Front */
	+10.0f, -10.0f, +10.0f,
	+10.0f, +10.0f, +10.0f,
	-10.0f, -10.0f, +10.0f, 
	-10.0f, +10.0f, -10.0f, /* Back */
	+10.0f, -10.0f, -10.0f,
	+10.0f, +10.0f, -10.0f,
	-10.0f, -10.0f, -10.0f,													 
	+10.0f, -10.0f, +10.0f, /* Right */
	+10.0f, +10.0f, -10.0f,
	+10.0f, +10.0f, +10.0f,
	+10.0f, -10.0f, -10.0f,								 	
	-10.0f, -10.0f, +10.0f, /* Left */
	-10.0f, +10.0f, -10.0f,
	-10.0f, +10.0f, +10.0f,
	-10.0f, -10.0f, -10.0f,						 
	-10.0f, -10.0f, +10.0f, /* Bottom */
	+10.0f, -10.0f, -10.0f,
	+10.0f, -10.0f, +10.0f,
	-10.0f, -10.0f, -10.0f,
	-10.0f, +10.0f, +10.0f, /* Top */
	+10.0f, +10.0f, -10.0f,
	+10.0f, +10.0f, +10.0f,
	-10.0f, +10.0f, -10.0f,
};
static const float cube_color[] = /* almost random colors */
{
	0.7f, 0.1f, 0.2f, 0.0f, 	0.8f, 0.9f, 0.3f, 0.0f,		0.4f, 1.0f, 0.5f, 0.0f,		0.0f, 0.6f, 0.1f, 0.0f,
	0.8f, 0.2f, 0.3f, 0.0f,		0.9f, 1.0f, 0.4f, 0.0f,		0.5f, 0.0f, 0.6f, 0.0f,		0.1f, 0.7f, 0.2f, 0.0f,
	0.9f, 0.3f, 0.4f, 0.0f,		1.0f, 0.0f, 0.5f, 0.0f,		0.6f, 0.1f, 0.7f, 0.0f,		0.2f, 0.8f, 0.3f, 0.0f,
	1.0f, 0.4f, 0.5f, 0.0f,		0.0f, 0.1f, 0.6f, 0.0f,		0.7f, 0.2f, 0.8f, 0.0f,		0.3f, 0.9f, 0.4f, 0.0f,
	0.0f, 0.5f, 0.6f, 0.0f,		0.1f, 0.2f, 0.7f, 0.0f,		0.8f, 0.3f, 0.9f, 0.0f,		0.4f, 1.0f, 0.5f, 0.0f,
	0.1f, 0.6f, 0.7f, 0.0f,		0.2f, 0.3f, 0.8f, 0.0f,		0.9f, 0.4f, 1.0f, 0.0f,		0.5f, 0.0f, 0.6f, 0.0f,
};
static const float cube_texture[] =
{
	0.0f, 0.0f,		1.0f, 1.0f,		1.0f, 0.0f,		0.0f, 1.0f,
	0.0f, 0.0f,		1.0f, 1.0f,		1.0f, 0.0f,		0.0f, 1.0f,
	0.0f, 0.0f,		1.0f, 1.0f,		1.0f, 0.0f,		0.0f, 1.0f,
	0.0f, 0.0f,		1.0f, 1.0f,		1.0f, 0.0f,		0.0f, 1.0f,
	0.0f, 0.0f,		1.0f, 1.0f,		1.0f, 0.0f,		0.0f, 1.0f,
	0.0f, 0.0f,		1.0f, 1.0f,		1.0f, 0.0f,		0.0f, 1.0f
};
static const float cube_normal[] =
{
	0.0f, 0.0f,	+1.0f,	0.0f, 0.0f,	+1.0f,	0.0f, 0.0f,	+1.0f,	0.0f, 0.0f,	+1.0f,
	0.0f, 0.0f,	-1.0f,	0.0f, 0.0f,	-1.0f,	0.0f, 0.0f,	-1.0f,	0.0f, 0.0f,	-1.0f,
	+1.0f, 0.0f, 0.0f,	+1.0f, 0.0f, 0.0f,	+1.0f, 0.0f, 0.0f,	+1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,	-1.0f, 0.0f, 0.0f,	-1.0f, 0.0f, 0.0f,	-1.0f, 0.0f, 0.0f,
	0.0f, -1.0f, 0.0f,	0.0f, -1.0f, 0.0f,	0.0f, -1.0f, 0.0f,	0.0f, -1.0f, 0.0f,
	0.0f, +1.0f, 0.0f,	0.0f, +1.0f, 0.0f,	0.0f, +1.0f, 0.0f,	0.0f, +1.0f, 0.0f,
};

static const unsigned char cube_index_buffer[] = {
	 0, 3, 1, 2, 0, 1,	/* Front */
	 6, 5, 4, 5, 7, 4,	/* Back */
	 8,11, 9,10, 8, 9,	/* Right */
	15,12,13,12,14,13,	/* Left */
	16,19,17,18,16,17,	/* Bottom */
	23,20,21,20,22,21	/* Top */
};
