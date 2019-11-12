#include "Chunk.h"

#include <cmath>
#include <map>
#include <thread>

#include "SurfaceTool.hpp"

#include "../SurfFaceEdge.h"
#include "ChunkMap.h"

//#include "../timer.h"

/*------------------------------------------------------------------------------
        VoxelInstance
------------------------------------------------------------------------------*/

// Copy constructor
VoxelInstance::VoxelInstance(VoxelInstance& origin){
    blockData = BlockDatabase::getSingleton()->getBlock(origin.blockData->blockID);
    // Make sure the copy has any special values set
    *blockData = *origin.blockData;

    flags = origin.flags;
    level = origin.level;
    center = origin.center;
    subVoxels = origin.subVoxels;
    //parent = origin.parent;
    map = origin.map;
}

// Move constructor
VoxelInstance::VoxelInstance(VoxelInstance&& origin){
    blockData = origin.blockData;
    origin.blockData = nullptr;

    flags = origin.flags;
    level = origin.level;
    center = origin.center;
    subVoxels = origin.subVoxels;
    //parent = origin.parent;
    map = origin.map;
}

// Function which recursiveley sets up an octree of blocks
void VoxelInstance::init(int level /*= SUBCHUNK_LEVELS*/, bool originalCall /*= true*/){
    this->level = level; // Mark which sublevel this instance is

    // If there is data to initalize and we aren't dealing with leaf nodes...
    if(level > 0){
        subVoxels = new VoxelInstance [8]; // Create the next sublevels

        // Distribute the blocks over the child sublevels
        for(int i = 0; i < 8; i++){
            subVoxels[i].map = map;

            //subVoxels[i].parent = this; // Mark this sublevel as the parent of the child sublevels
            subVoxels[i].init(level - 1, false);
        }
    // If we are dealing with leaf nodes... load an air block
    } else
        blockData = BlockDatabase::getSingleton()->getBlock(Blocks::AIR);

    #warning not working?
    // If this was the original call recalculate
    if(originalCall)
        recalculate();
}

// Function which merges sublevels containing all of the same blockID into the same level
bool VoxelInstance::prune(){
    // Variable which ensures that small data changes not large enouph to change the mathematical
    // mode of a sublevel don't get pruned
    bool canPrune = true;
    if(level > 1 && subVoxels)
        for(size_t i = 0; i < 8; i++)
            if(!subVoxels[i].prune())
                canPrune = false;

    size_t count = 0;	// Number of times the most common blockID appears in the sublevels
    bool unpruneableFeatures = false; // Variable tracking if there are any unprunebale features
    if(subVoxels){ // Make sure there are sublevels before finding the mode of the sublevels
        struct CountHolder{
                size_t count;
                BlockData* d;
        };
        std::map<Identifier, CountHolder> m; // Map used to sort blockIDs
        // Store the blockIDs sorted by occurence in a map
        for(size_t i = 0; i < 8; i++) {
            Identifier key = subVoxels[i].blockData->blockID;
            std::map<Identifier, CountHolder>::iterator it = m.find(key);
            if(it == m.end())
                m.insert(std::make_pair(key, (CountHolder){1, subVoxels[i].blockData}));
            else
                it->second.count++;

            // If this subVoxels is unpruneable, mark that we can't prune
            unpruneableFeatures = unpruneableFeatures || subVoxels[i].blockData->hasUnprunableFeature();
        }
        // Find the most common blockID in the sublevels and store it as this level's blockID
        Identifier finalID; // Variable storing the blockID which appears most
        BlockData* finalData;
        for(auto& it: m)
            if(it.second.count > count) {
                count = it.second.count;
                finalID = it.first;
                finalData = it.second.d;
            }
        if(blockData) delete blockData;
        // Make sure block data is the correct type
        blockData = BlockDatabase::getSingleton()->getBlock(finalID);
        *blockData = *finalData; // TODO: Overload assignment opperator in BlockData if issues emerge
    } else
        return true; // If we have already pruned this branch we are safe to prune higher

    // If the sublevels all have the same blockID, they can be removed
    if(count == 8 && canPrune){
        delete [] subVoxels;
        subVoxels = nullptr;
        return !unpruneableFeatures; // We are safe to prune higher up (assuming no unprunable features)
    }
    return false; // We are not safe to prune higher up
}

// Function which takes a pruned tree and rebuilds the lower levels of the tree down to the block level
// The purpose of this function is to rebuild a branch of the tree when we need to modify a blockID in that branch
void VoxelInstance::unprune(bool originalCall /*= true*/){
    // If we don't have children and we aren't at the block level
    if(level > 0 && !subVoxels){
        subVoxels = new VoxelInstance [8]; // Create the next sublevels

        // Pass down the block data to all the new children and unprune them
        for(int i = 0; i < 8; i++){
            //subVoxels[i].parent = this;
            subVoxels[i].map = map;
            subVoxels[i].level = level - 1;

            if(subVoxels[i].blockData) delete subVoxels[i].blockData;
            // Make sure block data is the correct type
            subVoxels[i].blockData = BlockDatabase::getSingleton()->getBlock(blockData->blockID);
            *subVoxels[i].blockData = *blockData;

            subVoxels[i].unprune(false);
        }
    // If we do have children and we aren't at the block level
    } else if (level > 0)
        // Unprune the children
        for(int i = 0; i < 8; i++)
            subVoxels[i].unprune(false);

    // If this is the original function call... recalculate
    if(originalCall)
        recalculate();
}

// Function which given an arbitrary point in 3D space within the voxel
// finds the subvoxel of the requested <lvl> which contains the point
VoxelInstance* VoxelInstance::find(int lvl, Vector3& position){
    // If we are at the desired level... return a reference to this node
    if(lvl == level)
        return this;

    // If this node has children...
    if(subVoxels){
        for(int i = 0; i < 8; i++){
            //gout << level << " - " << subVoxels[i].center << " <- " << position << endl;
            // Check if each child encloses the search position...
            if(subVoxels[i].within(position))
                // If it does recursively search into that child
                return subVoxels[i].find(lvl, position);
        }
    } else
        // If we are at a leaf node... return a reference to this node
        return this;
    return nullptr;
}

// Function which determines if an arbitrary point in space is within this voxel
bool VoxelInstance::within(Vector3& position){
    // Calculate the "radius" of the cube based on its level
    float radius = pow(2, level - 1);
    if(radius < .5) radius = .5;
    // Check if the arbitrary point is within those radius
    return !(position.x <= center.x - radius || position.x >= center.x + radius
        || position.y <= center.y - radius || position.y >= center.y + radius
        || position.z <= center.z - radius || position.z >= center.z + radius);
}

// Function which gets a single face disregarding visibility
Face VoxelInstance::getFace(Direction d){
    // variable storing the "radius" of the voxel
    float bounds = pow(2, level - 1);
    switch(d){
    case TOP:
        return Face(center + Vector3(-bounds, bounds, bounds),
            center + Vector3(bounds, bounds, bounds),
            center + Vector3(bounds, bounds, -bounds),
            center + Vector3(-bounds, bounds, -bounds), blockData->blockID).reverse();
    case BOTTOM:
        return Face(center + Vector3(-bounds, -bounds, bounds),
            center + Vector3(bounds, -bounds, bounds),
            center + Vector3(bounds, -bounds, -bounds),
            center + Vector3(-bounds, -bounds, -bounds), blockData->blockID);
    case NORTH:
        return Face(center + Vector3(bounds, -bounds, bounds),
            center + Vector3(bounds, bounds, bounds),
            center + Vector3(bounds, bounds, -bounds),
            center + Vector3(bounds, -bounds, -bounds), blockData->blockID);
    case SOUTH:
        return Face(center + Vector3(-bounds, -bounds, bounds),
            center + Vector3(-bounds, bounds, bounds),
            center + Vector3(-bounds, bounds, -bounds),
            center + Vector3(-bounds, -bounds, -bounds), blockData->blockID).reverse();
    case EAST:
        return Face(center + Vector3(-bounds, bounds, bounds),
            center + Vector3(bounds, bounds, bounds),
            center + Vector3(bounds, -bounds, bounds),
            center + Vector3(-bounds, -bounds, bounds), blockData->blockID);
    case WEST:
        return Face(center + Vector3(-bounds, bounds, -bounds),
            center + Vector3(bounds, bounds, -bounds),
            center + Vector3(bounds, -bounds, -bounds),
            center + Vector3(-bounds, -bounds, -bounds), blockData->blockID).reverse();
    }
}


//Function which gets the visible faces from a voxel instance
void VoxelInstance::getFaces(std::vector<Face>& out){
    if(blockData->checkFlag(BlockData::INVISIBLE))
        return;
    // Lambda which determines if the array contains a face already
    auto notHas = [&out](Face& what){
        for(const Face& f: out)
            if(f == what)
                return false;
        return true;
    };
    //auto notHasR = [notHas](Face&& what){ return notHas(what); };
    if(checkFlag(TOP_VISIBLE)){
        Face f = getFace(TOP);
        //if(notHas(f) && notHasR(f.reverse())) out.push_back(f);
        if(notHas(f)) out.push_back(f);
    }
    if(checkFlag(BOTTOM_VISIBLE)){
        Face f = getFace(BOTTOM);
        //if(notHas(f) && notHasR(f.reverse())) out.push_back(f);
        if(notHas(f)) out.push_back(f);
    }
    if(checkFlag(NORTH_VISIBLE)){
        Face f = getFace(NORTH);
        //if(notHas(f) && notHasR(f.reverse())) out.push_back(f);
        if(notHas(f)) out.push_back(f);
    }
    if(checkFlag(SOUTH_VISIBLE)){
        Face f = getFace(SOUTH);
        //if(notHas(f) && notHasR(f.reverse())) out.push_back(f);
        if(notHas(f)) out.push_back(f);
    }
    if(checkFlag(EAST_VISIBLE)){
        Face f = getFace(EAST);
        //if(notHas(f) && notHasR(f.reverse())) out.push_back(f);
        if(notHas(f)) out.push_back(f);
    }
    if(checkFlag(WEST_VISIBLE)){
        Face f = getFace(WEST);
        //if(notHas(f) && notHasR(f.reverse())) out.push_back(f);
        if(notHas(f)) out.push_back(f);
    }
}

// Function which recursively calculates the center of all of the sub voxels
void VoxelInstance::calculateCenters(){
    if(subVoxels){
        // TODO: check if there is a bug here
        //#warning check if there is a bug here
        float change = .5 * pow(2, level-1);

        subVoxels[0].center = center + Vector3 {change, change, change};
        subVoxels[1].center = center + Vector3 {change, change, -change};
        subVoxels[2].center = center + Vector3 {-change, change, -change};
        subVoxels[3].center = center + Vector3 {-change, change, change};
        subVoxels[4].center = center + Vector3 {change, -change, change};
        subVoxels[5].center = center + Vector3 {change, -change, -change};
        subVoxels[6].center = center + Vector3 {-change, -change, -change};
        subVoxels[7].center = center + Vector3 {-change, -change, change};

        for(int i = 0; i < 8; i++)
            subVoxels[i].calculateCenters();
    }
}

// Function which recursively calculates the visibility of all the subVoxels
void VoxelInstance::calculateVisibility(){
    #define setFlag(flag, vector)\
    { VoxelInstance* v = map->find(level, center + vector);\
    if(!v)\
        flags |= flag;\
        /*flags = flags & ~flag;*/\
    else if(v->blockData->checkFlag(BlockData::TRANSPARENT))\
        flags = flags | flag;\
    else\
        flags = flags & ~flag;\
    }
    // Distance to the center of the next voxel
    float distance = pow(2, level);
    // Top
    setFlag(VoxelInstance::TOP_VISIBLE, Vector3(0, distance, 0));
    // Bottom
    setFlag(VoxelInstance::BOTTOM_VISIBLE, Vector3(0, -distance, 0));
    // North
    setFlag(VoxelInstance::NORTH_VISIBLE, Vector3(distance, 0, 0));
    // South
    setFlag(VoxelInstance::SOUTH_VISIBLE, Vector3(-distance, 0, 0));
    // East
    setFlag(VoxelInstance::EAST_VISIBLE, Vector3(0, 0, distance));
    // West
    setFlag(VoxelInstance::WEST_VISIBLE, Vector3(0, 0, -distance));

    if(subVoxels)
        for(int i = 0; i < 8; i++)
            subVoxels[i].calculateVisibility();
}

// Function which loops over every block
void VoxelInstance::iterate(int lvl, IterationFunction func_ptr, int& index, bool threaded){
    // If we aren't at the requested level and we can go lower
    if(subVoxels && lvl != level)
        for(int i = 0; i < 8; i++)
            // Recursively itterate
            subVoxels[i].iterate(lvl, func_ptr, index, threaded);
    // If we are running threaded, create a new thread for the function
    #warning optimize?
    else if(threaded)
        std::thread(func_ptr, this, index++).detach();
    // Otherwise just call the function
    else
        func_ptr(this, index++);
}

// Function which gets all of the faces in one layer coming from a specified direction
std::vector<Face> VoxelInstance::getLayerFaces(const Direction direction, const int whichLevel){
    // Get all of the faces
    std::vector<Face> faces;
    iterate(BLOCK_LEVEL, [&faces](VoxelInstance* v, int) {
        v->getFaces(faces);
    });

    // Variable storing the eventual output
    std::vector<Face> out;

    // Variable storing the coordinate in world space to compare verticies too
    float match;
    // Variable storing the desired normal of all of the faces
    Vector3 normal;
    // Calculate the values for <normal> and <match>
    switch (direction){
    case TOP:
        match = center.y + CHUNK_DIMENSIONS / 2 - whichLevel;
        normal = Vector3(0, 1, 0);
        break;
    case BOTTOM:
        match = center.y - CHUNK_DIMENSIONS / 2 + whichLevel;
        normal = Vector3(0, -1, 0);
        break;
    case NORTH:
        match = center.x + CHUNK_DIMENSIONS / 2 - whichLevel;
        normal = Vector3(1, 0, 0);
        break;
    case SOUTH:
        match = center.x - CHUNK_DIMENSIONS / 2 + whichLevel;
        normal = Vector3(-1, 0, 0);
        break;
    case EAST:
        match = center.z + CHUNK_DIMENSIONS / 2 - whichLevel;
        normal = Vector3(0, 0, 1);
        break;
    case WEST:
        match = center.z - CHUNK_DIMENSIONS / 2 + whichLevel;
        normal = Vector3(0, 0, -1);
        break;
    }

    #define check(axis)\
        /* For each face */\
        for(Face& f: faces)\
            /* Check if three of it's points are in the plane (assuming square faces don't oddly tilt one verticie) */\
            if(f.a.point.axis == match && f.b.point.axis == match && f.c.point.axis == match)\
                /* Check if the face is oriented in the correct direction */\
                if(f.normal == normal)\
                    out.push_back(f)

    // Add all of the faces in the desired plane with the correct orientation to the output...
    if(direction == NORTH || direction == SOUTH) {
        check(x);
    } else if(direction == TOP || direction == BOTTOM) {
        check(y);
    } else if(direction == EAST || direction == WEST)
        check(z);
    // Return the layer
    return out;
}

// Debug functions
int VoxelInstance::count(){
    int count = 0;
    if(subVoxels){
        for(size_t i = 0; i < 8; i++)
            count += subVoxels[i].count();
        return count + 1;
    }
    return 1;
}

String copy(wchar_t what, int times){
    std::wstringstream s;
    for(int i = 0; i < times; i++)
        s << what;
    return String(s.str().c_str());
}

String to_string(bool what){
    if (what)
        return "true";
    return "false";
}

String VoxelInstance::dump(){
    String out = copy('\t', SUBCHUNK_LEVELS - level) + to_string(level) + " - " + to_string(blockData->blockID) +
        " - " + to_string(flags) + " - " + to_string(blockData->checkFlag(BlockData::TRANSPARENT))+ " - {" + center + "}\n";
    if(subVoxels)
        for(size_t i = 0; i < 8; i++)
            out += subVoxels[i].dump();
    return out;
}

/*------------------------------------------------------------------------------
        Chunk
------------------------------------------------------------------------------*/
// Function which rebuild's the chunk's mesh at the desired <levelOfDetail>
void Chunk::rebuildMesh(int levelOfDetail){
    Surface surf;
    std::vector<Face> facesArr;
    iterate(levelOfDetail, [&facesArr](VoxelInstance* me, int) {
        me->getFaces(facesArr);
    });

    if(facesArr.size())
        for (Face& f: facesArr)
            surf.append(f.getSurface());
        //surf = Surface::fromContiguousCoplanarFaces(facesArr);

    set_mesh(surf.getMesh());
}

// Function which computes a wireframe version of the mesh
void Chunk::buildWireframe(int levelOfDetail){
    Surface surf;
    std::vector<Face> facesArr;
    iterate(levelOfDetail, [&facesArr](VoxelInstance* me, int) {
        me->getFaces(facesArr);
    });

    if(facesArr.size())
        for (Face& f: facesArr)
            surf.append(f.getSurface());
        //surf = Surface::fromContiguousCoplanarFaces(facesArr);

    for(int i = 0; i < surf.indecies.size(); i += 3){
    	SurfaceTool* line = SurfaceTool::_new();
    	Vector3 normal = surf.norms[surf.indecies[i]].normalized() / 10000;
        line->begin(Mesh::PRIMITIVE_LINES);
        line->add_vertex(surf.verts.read()[surf.indecies.read()[i]] + normal);
        line->add_vertex(surf.verts.read()[surf.indecies.read()[i + 1]] + normal);
        line->add_vertex(surf.verts.read()[surf.indecies.read()[i + 1]] + normal);
        line->add_vertex(surf.verts.read()[surf.indecies.read()[i + 2]] + normal);
        line->add_vertex(surf.verts.read()[surf.indecies.read()[i + 2]] + normal);
        line->add_vertex(surf.verts.read()[surf.indecies.read()[i]] + normal);
        MeshInstance* instance = MeshInstance::_new();
        instance->set_mesh(line->commit());
        this->add_child(instance);
    }
}
