var draw_mouse_pos = {
    canvas: null,
    gl: null,
    shaderProgram: null,

    init: function(canvas_name) {
        this.canvas = document.getElementById(canvas_name);
        this.gl = this.canvas.getContext('webgl');

        const gl = this.gl;


        /*=========================Shaders========================*/

        // vertex shader source code
        const vertCode =
            'attribute vec3 coordinates;' +

            'void main(void) {' +
            'gl_Position = vec4(coordinates, 1.0);' +
            'gl_PointSize = 10.0;' +
            '}';

        // Create a vertex shader object
        const vertShader = gl.createShader(gl.VERTEX_SHADER);

        // Attach vertex shader source code
        gl.shaderSource(vertShader, vertCode);

        // Compile the vertex shader
        gl.compileShader(vertShader);

        // fragment shader source code
        const fragCode =
            'void main(void) {' +
            ' gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);' +
            '}';

        // Create fragment shader object
        const fragShader = gl.createShader(gl.FRAGMENT_SHADER);

        // Attach fragment shader source code
        gl.shaderSource(fragShader, fragCode);

        // Compile the fragmentt shader
        gl.compileShader(fragShader);

        // Create a shader program object to store
        // the combined shader program
        this.shaderProgram = gl.createProgram();

        // Attach a vertex shader
        gl.attachShader(this.shaderProgram, vertShader); 

        // Attach a fragment shader
        gl.attachShader(this.shaderProgram, fragShader);

        // Link both programs
        gl.linkProgram(this.shaderProgram);
    },

    render: function(mouse_x, mouse_y) {
        const canvas = this.canvas;
        const gl = this.gl;

        // Use the combined shader program object
        gl.useProgram(this.shaderProgram);

        /*==========Defining and storing the geometry=======*/
        const vertices = [
            (mouse_x / canvas.width) * 2 - 1,
            -(mouse_y / canvas.height * 2 - 1),
            0,
        ];

        // Create an empty buffer object to store the vertex buffer
        const vertex_buffer = gl.createBuffer();

        //Bind appropriate array buffer to it
        gl.bindBuffer(gl.ARRAY_BUFFER, vertex_buffer);

        // Pass the vertex data to the buffer
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);

        // Unbind the buffer
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        /*======== Associating shaders to buffer objects ========*/

        // Bind vertex buffer object
        gl.bindBuffer(gl.ARRAY_BUFFER, vertex_buffer);

        // Get the attribute location
        const coord = gl.getAttribLocation(this.shaderProgram, "coordinates");

        // Point an attribute to the currently bound VBO
        gl.vertexAttribPointer(coord, 3, gl.FLOAT, false, 0, 0);

        // Enable the attribute
        gl.enableVertexAttribArray(coord);

        /*============= Drawing the primitive ===============*/
        
        // Draw the triangle
        gl.drawArrays(gl.POINTS, 0, 1);
    },
}
