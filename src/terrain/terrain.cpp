#include <terrain/terrain.h>


// PerlinNoiseGenerator Implementation
PerlinNoiseGenerator::PerlinNoiseGenerator(float frequency, int octaves, float persistence)
    : m_frequency(frequency)
    , m_octaves(octaves)
    , m_persistence(persistence)
    , m_seed(12345)
    , perlin(siv::PerlinNoise(m_seed)) {
}

void PerlinNoiseGenerator::generateHeightMap(std::vector<std::vector<float>>& heightMap) {
    int width = heightMap.size();
    int height = heightMap[0].size();
    
    for(int x = 0; x < width; ++x) {
        for(int z = 0; z < height; ++z) {
            float amplitude = 1.0f;
            float frequency = m_frequency;
            float noiseValue = 0.0f;
            float totalAmplitude = 0.0f;

            for(int i = 0; i < m_octaves; ++i) {
                noiseValue += perlin.noise2D(x * frequency, z * frequency) * amplitude;
                totalAmplitude += amplitude;
                amplitude *= m_persistence;
                frequency *= 2.0f;
            }

            heightMap[x][z] = noiseValue / totalAmplitude;
        }
    }
}

// FaultFormationGenerator Implementation
FaultFormationGenerator::FaultFormationGenerator(int iterations, float minDelta, float maxDelta)
    : m_iterations(iterations)
    , m_minDelta(minDelta)
    , m_maxDelta(maxDelta) {
}

void FaultFormationGenerator::createFault(std::vector<std::vector<float>>& heightMap) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    int width = heightMap.size();
    int height = heightMap[0].size();

    // Generate two random points to create a line
    float x1 = dis(gen) * width;
    float z1 = dis(gen) * height;
    float x2 = dis(gen) * width;
    float z2 = dis(gen) * height;

    // Calculate line parameters
    float a = z2 - z1;
    float b = -(x2 - x1);
    float c = x2 * z1 - x1 * z2;
    float normal = std::sqrt(a * a + b * b);

    // Random height displacement
    float displacement = m_minDelta + dis(gen) * (m_maxDelta - m_minDelta);

    // Apply displacement to all points based on which side of the line they're on
    for(int x = 0; x < width; ++x) {
        for(int z = 0; z < height; ++z) {
            float distance = (a * x + b * z + c) / normal;
            if(distance > 0) {
                heightMap[x][z] += displacement;
            } else {
                heightMap[x][z] -= displacement;
            }
        }
    }
}

void FaultFormationGenerator::generateHeightMap(std::vector<std::vector<float>>& heightMap) {
    // Initialize heightmap to 0
    for(auto& row : heightMap) {
        std::fill(row.begin(), row.end(), 0.0f);
    }

    // Apply fault formation multiple times
    for(int i = 0; i < m_iterations; ++i) {
        createFault(heightMap);
    }

    // Normalize heightmap to [-1, 1] range
    float minHeight = heightMap[0][0];
    float maxHeight = heightMap[0][0];

    // Find min and max heights
    for(const auto& row : heightMap) {
        for(float height : row) {
            minHeight = std::min(minHeight, height);
            maxHeight = std::max(maxHeight, height);
        }
    }

    // Normalize
    float range = maxHeight - minHeight;
    for(auto& row : heightMap) {
        for(float& height : row) {
            height = 2.0f * (height - minHeight) / range - 1.0f;
        }
    }
}


//Midpoint Displacement Implementation
MidpointDisplacementGenerator::MidpointDisplacementGenerator(float roughness, float initialDisplacement)
    : m_roughness(roughness)
    , m_initialDisplacement(initialDisplacement) {
}

float MidpointDisplacementGenerator::getAverageHeight(
    const std::vector<std::vector<float>>& heightMap, 
    int x, int z, 
    int size
) {
    float sum = 0.0f;
    int count = 0;
    int width = heightMap.size();

    // Check all four corners if they're within bounds
    if (x - size >= 0 && z - size >= 0) { 
        sum += heightMap[x - size][z - size]; 
        count++; 
    }
    if (x - size >= 0 && z + size < width) { 
        sum += heightMap[x - size][z + size]; 
        count++; 
    }
    if (x + size < width && z - size >= 0) { 
        sum += heightMap[x + size][z - size]; 
        count++; 
    }
    if (x + size < width && z + size < width) { 
        sum += heightMap[x + size][z + size]; 
        count++; 
    }

    return sum / count;
}

void MidpointDisplacementGenerator::diamondStep(
    std::vector<std::vector<float>>& heightMap, 
    int size, 
    float displacement
) {
    int half = size / 2;
    int width = heightMap.size();

    for (int x = half; x < width; x += size) {
        for (int z = half; z < width; z += size) {
            float average = getAverageHeight(heightMap, x, z, half);
            heightMap[x][z] = average + displacement * (((float)rand() / RAND_MAX) * 2 - 1);
        }
    }
}

void MidpointDisplacementGenerator::squareStep(
    std::vector<std::vector<float>>& heightMap, 
    int size, 
    float displacement
) {
    int half = size / 2;
    int width = heightMap.size();

    for (int x = 0; x < width; x += half) {
        for (int z = (x + half) % size; z < width; z += size) {
            float average = getAverageHeight(heightMap, x, z, half);
            heightMap[x][z] = average + displacement * (((float)rand() / RAND_MAX) * 2 - 1);
        }
    }
}

void MidpointDisplacementGenerator::generateHeightMap(std::vector<std::vector<float>>& heightMap) {
    int width = heightMap.size();
    
    // Initialize corners with random values
    heightMap[0][0] = ((float)rand() / RAND_MAX) * 2 - 1;
    heightMap[0][width-1] = ((float)rand() / RAND_MAX) * 2 - 1;
    heightMap[width-1][0] = ((float)rand() / RAND_MAX) * 2 - 1;
    heightMap[width-1][width-1] = ((float)rand() / RAND_MAX) * 2 - 1;

    // Main generation loop
    float displacement = m_initialDisplacement;
    for (int size = width - 1; size > 1; size /= 2) {
        diamondStep(heightMap, size, displacement);
        squareStep(heightMap, size, displacement);
        displacement *= pow(2, -m_roughness);
    }
}



// Terrain Implementation
Terrain::Terrain() = default;

Terrain::Terrain(float yScale, float yShift, int resolution,
                 int width, int height,
                 int octaves, float persistence, float frequency,
                 int iterations, float minDelta, float maxDelta,
                 float roughness, float initialDisplacement)
    : m_VAO(0)
    , m_VBO(0)
    , m_IBO(0)
    , m_width(width)
    , m_height(height)
    , m_yScale(yScale)
    , m_yShift(yShift)
    , m_resolution(resolution)
    , m_numStrips(0)
    , m_numTrisPerStrip(0)
    , m_octaves(octaves)
    , m_persistence(persistence)
    , m_frequency(frequency)
    , m_iterations(iterations)
    , m_minDelta(minDelta)
    , m_maxDelta(maxDelta)
    , m_roughness(roughness)
    , m_initialDisplacement(initialDisplacement)
    , heightMap(width, std::vector<float>(height)) {
    initializeGLBuffers();
}

void Terrain::setTerrainGenerator(GenerationType type) {
    switch(type) {
        case GenerationType::PERLIN_NOISE:
            m_currentGenerator = std::make_unique<PerlinNoiseGenerator>(m_frequency, m_octaves, m_persistence);
            break;
        case GenerationType::FAULT_FORMATION:
            m_currentGenerator = std::make_unique<FaultFormationGenerator>(m_iterations, m_minDelta, m_maxDelta);
            break;
        case GenerationType::MIDPOINT_DISPLACEMENT:
            m_currentGenerator = std::make_unique<MidpointDisplacementGenerator>(0.5f, 1.0f);
            break;
        default:
            throw std::runtime_error("Unknown terrain generation type");
    }
}

void Terrain::generateTerrain(GenerationType type) {
    setTerrainGenerator(type);
    
    if (!m_currentGenerator) {
        throw std::runtime_error("No terrain generator selected");
    }

    m_currentGenerator->generateHeightMap(heightMap);
    
    try {
        generateVertexArray();
        generateIndices();
        setupBuffers();
    } catch (const std::exception& e) {
        throw;
    }
}

void Terrain::initializeGLBuffers() {
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_IBO);
}

void Terrain::generateVertexArray() {
    m_vertexArray.clear();
    m_vertexArray.reserve(m_width * m_height * 5);

    for(int x = 0; x < m_height; x++) {
        for(int z = 0; z < m_width; z++) {
            float height = heightMap[x][z] * m_yScale - m_yShift;            
            m_vertexArray.push_back(-m_height/2.0f + x);
            m_vertexArray.push_back(height);
            m_vertexArray.push_back(-m_width/2.0f + z);
            m_vertexArray.push_back(static_cast<float>(x) / (m_height - 1) * 10);
            m_vertexArray.push_back(static_cast<float>(z) / (m_width - 1) * 10);
        }
    }
}

void Terrain::generateIndices() {
    m_indices.clear();
    m_indices.reserve((m_height - 1) * m_width * 2);

    for(int i = 0; i < m_height-1; i++) {
        for(int j = 0; j < m_width; j++) {
            for(int k = 0; k < 2; k++) {
                m_indices.push_back(j + m_width * (i + k));
            }
        }
    }
    
    m_numStrips = (m_height-1)/m_resolution;
    m_numTrisPerStrip = (m_width/m_resolution)*2-2;
}

void Terrain::setupBuffers() {
    if (m_vertexArray.empty() || m_indices.empty()) {
        throw std::runtime_error("No vertex or index data to upload to GPU");
    }

    glBindVertexArray(m_VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertexArray.size() * sizeof(float), m_vertexArray.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);    

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);       

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
    
    

}

void Terrain::initTexture(Shader& shader, const std::vector<const char*>& texturePaths) {
    unsigned int textures[texturePaths.size()];  
    glGenTextures(texturePaths.size(), textures);
    shader.use();

    for(int i=0; i< texturePaths.size(); i++){
        glActiveTexture(GL_TEXTURE0 + i); //
        glBindTexture(GL_TEXTURE_2D, textures[i]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        int widthImg, heightImg, nrChannels;
        unsigned char *data = stbi_load(texturePaths[i], &widthImg, &heightImg, &nrChannels, 0);

        if(data){
            std::cout<<"yes texture"<<std::to_string(i)<<std::endl;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, widthImg, heightImg, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else{
            std::cout<<"no texture"<<std::to_string(i)<<std::endl;
        }    

        stbi_image_free(data);  
        shader.setInt("ourTexture"+std::to_string(i+1), i);
    }

}

void Terrain::render() const {
    glBindVertexArray(m_VAO);
    
    for(unsigned strip = 0; strip < m_numStrips; strip++) {
        
        glDrawElements(
            GL_TRIANGLE_STRIP,
            m_numTrisPerStrip + 2,
            GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned int) * (m_numTrisPerStrip + 2) * strip)
        );
    }
}

Terrain::~Terrain() {
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_IBO) glDeleteBuffers(1, &m_IBO);
}