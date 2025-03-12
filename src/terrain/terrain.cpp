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
    
    // Create domain warping noise for more natural terrain features
    std::vector<std::vector<float>> warpX(width, std::vector<float>(height, 0.0f));
    std::vector<std::vector<float>> warpZ(width, std::vector<float>(height, 0.0f));
    
    // Generate domain warping values
    const float warpStrength = 10.0f;
    for(int x = 0; x < width; ++x) {
        for(int z = 0; z < height; ++z) {
            float wx = perlin.noise2D(x * 0.01f, z * 0.01f);
            float wz = perlin.noise2D(x * 0.01f + 100.0f, z * 0.01f + 100.0f);
            warpX[x][z] = wx * warpStrength;
            warpZ[x][z] = wz * warpStrength;
        }
    }
    
    // Ridged multifractal parameters
    const float ridgeOffset = 1.0f;
    const float ridginess = 0.5f; // 0 = normal perlin, 1 = full ridge
    
    // Apply fractal noise with domain warping
    for(int x = 0; x < width; ++x) {
        for(int z = 0; z < height; ++z) {
            float amplitude = 1.0f;
            float frequency = m_frequency;
            float noiseValue = 0.0f;
            float totalAmplitude = 0.0f;
            
            // Variable to accumulate turbulence for domain warping
            float turbulence = 0.0f;

            for(int i = 0; i < m_octaves; ++i) {
                // Apply domain warping for more natural terrain flow
                float wx = x + warpX[x][z] * (i + 1) * 0.1f;
                float wz = z + warpZ[x][z] * (i + 1) * 0.1f;
                
                // Calculate basic noise
                float n = perlin.noise2D(wx * frequency, wz * frequency);
                
                // Apply ridged multifractal for mountains
                if (i < m_octaves / 2) {
                    n = ridgeOffset - std::abs(n);
                    n = std::pow(n, 2.0f); // Sharpen ridges
                }
                
                // Worley noise for the lower frequencies to create erosion-like features
                if (i >= m_octaves / 2) {
                    float worleyScale = 0.5f;
                    float cellSize = 1.0f / (frequency * 2.0f);
                    float worleyX = std::floor(wx * frequency) * cellSize;
                    float worleyZ = std::floor(wz * frequency) * cellSize;
                    float minDist = 1.0f;
                    
                    // Simple Worley noise calculation
                    for (int ox = -1; ox <= 1; ox++) {
                        for (int oz = -1; oz <= 1; oz++) {
                            float cellX = worleyX + ox * cellSize;
                            float cellZ = worleyZ + oz * cellSize;
                            
                            // Get random point in cell
                            float randomX = cellX + cellSize * perlin.noise2D(cellX * 1000, cellZ * 1000);
                            float randomZ = cellZ + cellSize * perlin.noise2D(cellX * 1000 + 50, cellZ * 1000 + 50);
                            
                            float dx = wx * frequency - randomX;
                            float dz = wz * frequency - randomZ;
                            float dist = std::sqrt(dx * dx + dz * dz);
                            
                            minDist = std::min(minDist, dist);
                        }
                    }
                    
                    // Blend Perlin with Worley
                    n = n * (1.0f - worleyScale) + minDist * worleyScale;
                }
                
                // Apply turbulence from previous octaves for more realistic variation
                if (i > 0) {
                    n += turbulence * 0.1f * i;
                }
                
                // Accumulate noise with amplitude
                noiseValue += n * amplitude;
                totalAmplitude += amplitude;
                
                // Update turbulence
                turbulence = noiseValue;
                
                // Update amplitude and frequency for next octave
                amplitude *= m_persistence;
                frequency *= 2.0f;
            }
            
            // Normalize and store
            heightMap[x][z] = noiseValue / totalAmplitude;
            
            // Apply additional terrain shaping
            float plateauAmount = 0.3f; // 0 = no plateaus, 1 = full plateaus
            if (heightMap[x][z] > 0.7f) {
                // Create plateaus on high areas
                float h = heightMap[x][z];
                float t = (h - 0.7f) / 0.3f; // Normalize to 0-1 for plateau range
                heightMap[x][z] = h * (1.0f - plateauAmount * t) + 0.8f * plateauAmount * t;
            }
            else if (heightMap[x][z] < -0.3f) {
                // Flatten lowlands slightly
                float h = heightMap[x][z];
                float t = (-h - 0.3f) / 0.7f; // Normalize to 0-1 for lowland range
                heightMap[x][z] = h * (1.0f - plateauAmount * t) - 0.4f * plateauAmount * t;
            }
        }
    }
    
    // Apply thermal erosion simulation (simplified)
    const int erosionIterations = 3;
    const float talusAngle = 0.05f; // Talus angle in heightmap units
    
    for (int iter = 0; iter < erosionIterations; iter++) {
        std::vector<std::vector<float>> heightMapCopy = heightMap;
        
        for(int x = 1; x < width - 1; ++x) {
            for(int z = 1; z < height - 1; ++z) {
                // Check all 8 neighbors
                float maxDiff = 0.0f;
                int maxX = x, maxZ = z;
                
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dz = -1; dz <= 1; dz++) {
                        if (dx == 0 && dz == 0) continue;
                        
                        float diff = heightMapCopy[x][z] - heightMapCopy[x+dx][z+dz];
                        if (diff > maxDiff) {
                            maxDiff = diff;
                            maxX = x + dx;
                            maxZ = z + dz;
                        }
                    }
                }
                
                // If slope is too steep, erode
                if (maxDiff > talusAngle) {
                    float transfer = (maxDiff - talusAngle) * 0.5f;
                    heightMap[x][z] -= transfer;
                    heightMap[maxX][maxZ] += transfer;
                }
            }
        }
    }
}


// FaultFormationGenerator Implementation
FaultFormationGenerator::NoiseGenerator::NoiseGenerator(int seed) 
    : m_seed(seed) {
}

void FaultFormationGenerator::NoiseGenerator::setSeed(int seed) { 
    m_seed = seed; 
}

float FaultFormationGenerator::NoiseGenerator::getNoise(float x, float y) {
    static siv::PerlinNoise perlin(m_seed);
    return perlin.noise2D(x, y);
}

float FaultFormationGenerator::NoiseGenerator::getOctaveNoise(float x, float y, int octaves, float persistence) {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;
    
    for(int i = 0; i < octaves; i++) {
        total += getNoise(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    
    return total / maxValue;
}

// FaultFormationGenerator Implementation
FaultFormationGenerator::FaultFormationGenerator(int iterations, float minDelta, float maxDelta)
    : m_iterations(iterations)
    , m_minDelta(minDelta)
    , m_maxDelta(maxDelta)
    , m_terrainType(GENERIC)
    , m_noise(12345) {
}

void FaultFormationGenerator::setTerrainType(FaultFormationGenerator::TerrainType type) {
    m_terrainType = type;
}

float FaultFormationGenerator::calculateDisplacement(float iteration, float totalIterations) {
    // Exponentially decrease displacement as iterations progress
    float progress = iteration / totalIterations;
    float factor = std::exp(-4.0f * progress);
    
    // Interpolate between max and min delta based on the factor
    return m_minDelta + (m_maxDelta - m_minDelta) * factor;
}

std::pair<glm::vec2, glm::vec2> FaultFormationGenerator::generateFaultPoints(int width, int height, float iteration) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    // Calculate center point and radius
    float centerX = width * 0.5f;
    float centerY = height * 0.5f;
    float radius = std::min(width, height) * 0.5f;
    
    // Add directional bias based on terrain type
    float directionalBias = m_terrainType == MOUNTAIN_RANGE ? 0.7f : 0.0f;
    float preferredAngle = M_PI * 0.25f; // 45 degrees for example
    
    // Generate angle for first point with potential bias
    float angle1;
    if (dis(gen) < directionalBias) {
        // Apply bias toward preferred direction
        angle1 = preferredAngle + (dis(gen) - 0.5f) * M_PI * 0.3f;
    } else {
        angle1 = dis(gen) * 2.0f * M_PI;
    }
    
    float x1 = centerX + radius * std::cos(angle1);
    float y1 = centerY + radius * std::sin(angle1);
    
    // Generate angle for second point (opposite side with variation)
    float angle2 = angle1 + M_PI + (dis(gen) - 0.5f) * M_PI * 0.5f;
    float x2 = centerX + radius * std::cos(angle2);
    float y2 = centerY + radius * std::sin(angle2);
    
    return std::make_pair(glm::vec2(x1, y1), glm::vec2(x2, y2));
}

void FaultFormationGenerator::createFault(std::vector<std::vector<float>>& heightMap, float iteration) {
    int width = heightMap.size();
    int height = heightMap[0].size();
    
    // Generate fault line points
    auto [p1, p2] = generateFaultPoints(width, height, iteration);
    
    // Calculate line parameters (ax + by + c = 0)
    float a = p2.y - p1.y;
    float b = -(p2.x - p1.x);
    float c = p2.x * p1.y - p1.x * p2.y;
    float normal = std::sqrt(a * a + b * b);
    
    // Calculate displacement based on current iteration
    float displacement = calculateDisplacement(iteration, m_iterations);
    
    // Apply displacement with smooth falloff
    const float falloffDistance = std::min(width, height) * 0.1f;
    
    // Use existing noise generator for fault perturbation
    m_noise.setSeed(static_cast<int>(iteration * 1000));
    
    #pragma omp parallel for collapse(2)
    for(int x = 0; x < width; ++x) {
        for(int y = 0; y < height; ++y) {
            // Apply noise to perturb the distance calculation
            float noiseValue = m_noise.getNoise(static_cast<float>(x) * 0.01f, 
                                               static_cast<float>(y) * 0.01f) * 10.0f;
            
            // Perturbed distance calculation
            float distance = (a * x + b * y + c) / normal + noiseValue;
            
            // Calculate falloff factor with variable falloff distance
            float localFalloffDistance = falloffDistance * (1.0f + 0.3f * 
                m_noise.getNoise(static_cast<float>(x) * 0.005f, static_cast<float>(y) * 0.005f));
            float falloff = 1.0f;
            
            if(std::abs(distance) < localFalloffDistance) {
                falloff = std::abs(distance) / localFalloffDistance;
                falloff = 0.5f + 0.5f * std::cos(falloff * M_PI);
            }
            
            // Apply displacement with falloff
            if(distance > 0) {
                heightMap[x][y] += displacement * falloff;
            } else {
                heightMap[x][y] -= displacement * falloff;
            }
        }
    }
}

// Add an erosion pass after generating the base heightmap
void FaultFormationGenerator::applySimpleErosion(std::vector<std::vector<float>>& heightMap, int iterations) {
    int width = heightMap.size();
    int height = heightMap[0].size();
    
    std::vector<std::vector<float>> tempMap = heightMap;
    
    for (int iter = 0; iter < iterations; ++iter) {
        #pragma omp parallel for collapse(2)
        for(int x = 1; x < width - 1; ++x) {
            for(int y = 1; y < height - 1; ++y) {
                // Simple thermal erosion - material moves from higher to lower cells
                float current = heightMap[x][y];
                float lowestNeighbor = current;
                int lowestX = x, lowestY = y;
                
                // Check all 8 neighbors
                for (int nx = x-1; nx <= x+1; ++nx) {
                    for (int ny = y-1; ny <= y+1; ++ny) {
                        if (nx == x && ny == y) continue;
                        
                        if (heightMap[nx][ny] < lowestNeighbor) {
                            lowestNeighbor = heightMap[nx][ny];
                            lowestX = nx;
                            lowestY = ny;
                        }
                    }
                }
                
                // If current cell is higher than its lowest neighbor
                if (current > lowestNeighbor) {
                    float diff = current - lowestNeighbor;
                    float amount = std::min(diff * 0.1f, 0.05f); // Limit erosion rate
                    
                    tempMap[x][y] -= amount;
                    tempMap[lowestX][lowestY] += amount;
                }
            }
        }
        
        heightMap = tempMap;
    }
}

// Add detail with multiple octaves of noise
void FaultFormationGenerator::addDetailNoise(std::vector<std::vector<float>>& heightMap, float intensity) {
    int width = heightMap.size();
    int height = heightMap[0].size();
    
    // Set different seed for detail noise
    m_noise.setSeed(42);
    
    #pragma omp parallel for collapse(2)
    for(int x = 0; x < width; ++x) {
        for(int y = 0; y < height; ++y) {
            float detail = m_noise.getOctaveNoise(
                static_cast<float>(x) * 0.01f, 
                static_cast<float>(y) * 0.01f,
                3,  // 3 octaves
                0.5f  // persistence
            );
            
            // Apply detail noise with varying intensity based on height
            // More detail at higher elevations (mountains) and less in valleys
            float heightFactor = 0.5f + 0.5f * heightMap[x][y]; // Map [-1,1] to [0,1]
            heightMap[x][y] += detail * intensity * heightFactor;
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
        createFault(heightMap, static_cast<float>(i));
    }
    
    // Apply different levels of detail
    addDetailNoise(heightMap, 0.1f);
    
    // Apply simple erosion simulation
    applySimpleErosion(heightMap, 5);
    
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
    
    // Normalize with non-linear mapping to emphasize terrain features
    float range = maxHeight - minHeight;
    for(auto& row : heightMap) {
        for(float& height : row) {
            // Normalize to [0, 1] first
            float normalizedHeight = (height - minHeight) / range;
            
            // Apply non-linear transformations to create more realistic height distributions
            // This creates more flat areas and steeper mountains
            if (normalizedHeight < 0.4f) {
                // Lower areas stay relatively flat
                normalizedHeight = normalizedHeight * 0.5f;
            } else if (normalizedHeight > 0.7f) {
                // Higher areas become steeper
                normalizedHeight = 0.7f + (normalizedHeight - 0.7f) * 1.5f;
            }
            
            // Convert back to [-1, 1] range
            height = normalizedHeight * 2.0f - 1.0f;
        }
    }
}


// MidpointDisplacementGenerator Implementation
MidpointDisplacementGenerator::MidpointDisplacementGenerator(float roughness, float initialDisplacement)
    : m_roughness(roughness)
    , m_initialDisplacement(initialDisplacement) {
    // Validate parameters
    if (roughness < 0.0f) {
        throw std::runtime_error("Roughness must be positive");
    }
}

int MidpointDisplacementGenerator::calcNextPowerOfTwo(int size) {
    int power = 1;
    while (power < size - 1) {
        power *= 2;
    }
    return power;
}

float MidpointDisplacementGenerator::randomFloatRange(float min, float max) {
    return min + (max - min) * (static_cast<float>(rand()) / RAND_MAX);
}

void MidpointDisplacementGenerator::diamondStep(
    std::vector<std::vector<float>>& heightMap, 
    int rectSize, 
    float curHeight
) {
    int halfRectSize = rectSize / 2;
    int width = heightMap.size();

    for (int y = 0; y < width; y += rectSize) {
        for (int x = 0; x < width; x += rectSize) {
            int nextX = (x + rectSize) % width;
            int nextY = (y + rectSize) % width;

            // Handle wrapping
            if (nextX < x) {
                nextX = width - 1;
            }
            if (nextY < y) {
                nextY = width - 1;
            }

            float topLeft = heightMap[y][x];
            float topRight = heightMap[y][nextX];
            float bottomLeft = heightMap[nextY][x];
            float bottomRight = heightMap[nextY][nextX];

            int midX = (x + halfRectSize) % width;
            int midY = (y + halfRectSize) % width;

            float randValue = randomFloatRange(-curHeight, curHeight);
            float midPoint = (topLeft + topRight + bottomLeft + bottomRight) / 4.0f;

            heightMap[midY][midX] = midPoint + randValue;
        }
    }
}

void MidpointDisplacementGenerator::squareStep(
    std::vector<std::vector<float>>& heightMap, 
    int rectSize, 
    float curHeight
) {
    /*                ----------------------------------
                      |                                |
                      |           PrevYCenter          |
                      |                                |
                      |                                |
                      |                                |
    ------------------CurTopLeft..CurTopMid..CurTopRight
                      |                                |
                      |                                |
       CurPrevXCenter CurLeftMid   CurCenter           |
                      |                                |
                      |                                |
                      CurBotLeft------------------------

       CurTopMid = avg(PrevYCenter, CurTopLeft, CurTopRight, CurCenter)
       CurLeftMid = avg(CurPrevXCenter, CurTopLeft, CurBotLeft, CurCenter)
    */

    int halfRectSize = rectSize / 2;
    int width = heightMap.size();

    for (int y = 0; y < width; y += rectSize) {
        for (int x = 0; x < width; x += rectSize) {
            int nextX = (x + rectSize) % width;
            int nextY = (y + rectSize) % width;

            // Handle wrapping
            if (nextX < x) {
                nextX = width - 1;
            }
            if (nextY < y) {
                nextY = width - 1;
            }

            int midX = (x + halfRectSize) % width;
            int midY = (y + halfRectSize) % width;
                
            int prevMidX = (x - halfRectSize + width) % width;
            int prevMidY = (y - halfRectSize + width) % width;

            float curTopLeft = heightMap[y][x];
            float curTopRight = heightMap[y][nextX];
            float curCenter = heightMap[midY][midX];
            float prevYCenter = heightMap[prevMidY][midX];
            float curBotLeft = heightMap[nextY][x]; 
            float prevXCenter = heightMap[midY][prevMidX];

            float curLeftMid = (curTopLeft + curCenter + curBotLeft + prevXCenter) / 4.0f + 
                               randomFloatRange(-curHeight, curHeight);
            float curTopMid = (curTopLeft + curCenter + curTopRight + prevYCenter) / 4.0f + 
                              randomFloatRange(-curHeight, curHeight);

            heightMap[y][midX] = curTopMid;
            heightMap[midY][x] = curLeftMid;
        }
    }
}

void MidpointDisplacementGenerator::generateHeightMap(std::vector<std::vector<float>>& heightMap) {
    int width = heightMap.size();
    
    // Calculate rectangle size (power of 2)
    int rectSize = calcNextPowerOfTwo(width);
    float curHeight = m_initialDisplacement;
    float heightReduce = std::pow(2.0f, -m_roughness);
    
    // Initialize corners with random values
    heightMap[0][0] = randomFloatRange(-curHeight, curHeight);
    heightMap[0][width-1] = randomFloatRange(-curHeight, curHeight);
    heightMap[width-1][0] = randomFloatRange(-curHeight, curHeight);
    heightMap[width-1][width-1] = randomFloatRange(-curHeight, curHeight);
    
    // Main generation loop
    while (rectSize > 0) {
        diamondStep(heightMap, rectSize, curHeight);
        squareStep(heightMap, rectSize, curHeight);
        
        rectSize /= 2;
        curHeight *= heightReduce;
    }
    
    // Normalize heightmap to [-1, 1] range
    float minHeight = heightMap[0][0];
    float maxHeight = heightMap[0][0];

    // Find min and max heights
    for (const auto& row : heightMap) {
        for (float height : row) {
            minHeight = std::min(minHeight, height);
            maxHeight = std::max(maxHeight, height);
        }
    }

    // Normalize
    float range = maxHeight - minHeight;
    for (auto& row : heightMap) {
        for (float& height : row) {
            height = 2.0f * (height - minHeight) / range - 1.0f;
        }
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
    , heightMap(width, std::vector<float>(height, 0.0f))
    , currentHeightMap(width, std::vector<float>(height, 0.0f))
    , heightMaps(static_cast<int>(GenerationType::COUNT),std::vector<std::vector<float>>(width, std::vector<float>(height, 0.0f))){
    initializeGLBuffers();
}

float Terrain::getYScale() const { return m_yScale; }
float Terrain::getYShift() const { return m_yShift; }
float Terrain::getheightMin() const { return m_heightMin; }
float Terrain::getheightMax() const { return m_heightMax; }

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

void Terrain::addedTerrain(){
    for (int i = 0; i < static_cast<int>(GenerationType::COUNT); ++i) {
        GenerationType type = static_cast<GenerationType>(i);

        setTerrainGenerator(type);  

        if (!m_currentGenerator) {
            throw std::runtime_error("No terrain generator selected");
        }

        m_currentGenerator->generateHeightMap(heightMap);
        for(int x=0; x<m_width; x++)
        {
            for(int z=0; z<m_height; z++){
                heightMaps[i][x][z]=heightMap[x][z]; 
            }
        }
    }

    maxMaps();

    try {
        generateVertexArray(currentHeightMap);
        generateIndices();
        setupBuffers();
    } catch (const std::exception& e) {
        throw;
    }    
}

void Terrain::addMaps(){
    for(int i =0; i< static_cast<int>(GenerationType::COUNT); i++){
        for(int x=0; x<m_width; x++)
        {
            for(int z=0; z<m_height; z++){
                currentHeightMap[x][z]+=heightMaps[i][x][z]; 
            }
        }
    }
}

void Terrain::maxMaps(){
    for(int i =0; i< static_cast<int>(GenerationType::COUNT); i++){
        for(int x=0; x<m_width; x++)
        {
            for(int z=0; z<m_height; z++){
                currentHeightMap[x][z]=std::max(currentHeightMap[x][z], heightMaps[i][x][z]); 
            }
        }
    }
}

void Terrain::weightedAddMaps(){
    std::vector<float> weights = {0.1f, 0.1f, 0.8f};
    for(int i =0; i< static_cast<int>(GenerationType::COUNT); i++){
        for(int x=0; x<m_width; x++)
        {
            for(int z=0; z<m_height; z++){
                currentHeightMap[x][z]+=weights[i] * heightMaps[i][x][z]; 
            }
        }
    }    
}


void Terrain::generateTerrain(GenerationType type) {
    setTerrainGenerator(type);
    
    if (!m_currentGenerator) {
        throw std::runtime_error("No terrain generator selected");
    }
    
    m_currentGenerator->generateHeightMap(heightMap);

    m_heightMin = heightMap[0][0];
    m_heightMax = heightMap[0][0];
    for (const auto& row : heightMap) {
        for (float height : row) {
            m_heightMin = std::min(m_heightMin, height);
            m_heightMax = std::max(m_heightMax, height);
        }
    }    

    
    try {
        generateVertexArray(heightMap);
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

void Terrain::generateVertexArray(std::vector<std::vector<float>>& heightMap) {
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