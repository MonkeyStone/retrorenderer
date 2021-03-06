The meshes are loaded from Wavefront .obj files. Mostly it uses standard 3D
rendering techniques, much like the ones that OpenGL uses. The things that are
done differently from normal are:

  * The images are supersampled at triple resolution after performing the
depth test but before doing lighting calculations. That is, the fragments are
rendered into buffers which hold the materials, depths, and surface normals,
and then each group of 3x3 fragments is combined into one super-fragment which
is the average of all of them. This step is necessary to avoid "jaggies" at
the edges of the faces.

  * In the diffuse shading step, after the dot product of the surface normal
and the light is calculated, it is rounded to one of several specific values
to produce a "banded" shading effect. Specifically, the code is:


	double bias = 0.2;

	double alignment = (dot(light.direction, normal) + bias) / (1 + bias);

	if (alignment > 0)

      	{

		// Make shading granular

		if (alignment < 0.6) alignment = 0.4;

          	else alignment = 0.8;

		// Diffuse lighting

		color = color + alignment * mat.diffuse * light.color;

		//(snip specular lighting code here)

	}


  * After the image has been shaded, the black outlines are added. A pixel is
changed to black if at least one of its neighbors is painted with a different
material, and that neighbor has a lower depth value than it does. This works
pretty well most of the time, but one must take care to paint each object in
the scene with a different material to make sure the outlines are drawn
properly. I unthinkingly put the same texture on both of the knight's boots
(in the second sample render) and there is no black outline between them in
some of the images.

  * Finally, the entire image is scaled up by a factor of 2. I did this by
hand on the images I posted to reddit -- the 3D engine doesn't do this.


