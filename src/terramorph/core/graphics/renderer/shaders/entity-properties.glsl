struct EntityPropertySet {
    uint position;
    float leftSizeFactor;
    float bottomSizeFactor;
    float rightSizeFactor;
    float topSizeFactor;
    uint textureIndex;
};

layout (std140) uniform EntityPropertySets {
    EntityPropertySet entityPropertySets[100];
};