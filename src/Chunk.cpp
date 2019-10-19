#include "Chunk.h"

#include <cmath>
#include <map>
#include <functional>

#include "SurfFaceEdge.h"

/*------------------------------------------------------------------------------
        VoxelInstance
------------------------------------------------------------------------------*/

// Copy constructor
VoxelInstance::VoxelInstance(VoxelInstance& origin){
    blockData = new BlockData();
    *blockData = *origin.blockData;

    flags = origin.flags;
    level = origin.level;
    center = origin.center;
    subVoxels = origin.subVoxels;
    parent = origin.parent;
}

// Move constructor
VoxelInstance::VoxelInstance(VoxelInstance&& origin){
    blockData = origin.blockData;
    origin.blockData = nullptr;

    flags = origin.flags;
    level = origin.level;
    center = origin.center;
    subVoxels = origin.subVoxels;
    parent = origin.parent;
}

// Function which recursiveley converts an array of blockIDs into an octree
void VoxelInstance::init(int blocks[], int level /*= SUBCHUNK_LEVELS*/, bool originalCall /*= true*/){
    this->level = level; // Mark which sublevel this instance is

    // If there is data to initalize and we aren't dealing with leaf nodes...
    if(blocks && level > 0){
            subVoxels = new VoxelInstance [8]; // Create the next sublevels

        // Determine how many blocks are in the source array at this level
        int numLeft = pow(2, level);
        numLeft *= numLeft * numLeft;

        // Distribute the blocks over the child sublevels
        for(int i = 0; i < 8; i++){
            subVoxels[i].parent = this; // Mark this sublevel as the parent of the child sublevels
            subVoxels[i].init(&blocks[i * (numLeft / 8)], level - 1, false);
        }
    // If we are dealing with leaf nodes...
    // TODO: load the block data from some block repository
    } else
        blockData->blockID = blocks[0];

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

    uint count = 0;	// Number of times the most common blockID appears in the sublevels
    if(subVoxels){ // Make sure there are sublevels before finding the mode of the sublevels
        struct CountHolder{
                uint count;
                BlockData* d;
        };
        std::map<uint, CountHolder> m; // Map used to sort blockIDs
        // Store the blockIDs sorted by occurence in a map
        for(size_t i = 0; i < 8; i++) {
            uint key = subVoxels[i].blockData->blockID;
            std::map<uint, CountHolder>::iterator it = m.find(key);
            if(it == m.end())
                m.insert(std::make_pair(key, (CountHolder){1, subVoxels[i].blockData}));
            else
                it->second.count++;
        }
        // Find the most common blockID in the sublevels and store it as this level's blockID
        for(auto& it: m)
            if(it.second.count > count) {
                count = it.second.count;
                // TODO: get from database?
                *blockData = *it.second.d;
            }
    } else
        return true; // If we have already pruned this branch we are safe to prune higher

    // If the sublevels all have the same blockID, they can be removed
    if(count == 8 && canPrune){
        delete [] subVoxels;
        subVoxels = nullptr;
        return true; // We are safe to prune higher up
    }
    return false; // We are not safe to prune higher up
}

// Function which takes a pruned tree and rebuilds the lower levels of the tree down to the block level
// The purpose of this function is to rebuild a branch of the tree when we need to modify a blockID in that branch
void VoxelInstance::unprune(bool originalCall /*= true*/){
    // If we don't have children and we aren't at the block level
    if(level > 0 && !subVoxels){
        subVoxels = new VoxelInstance [8]; // Create the next sublevels

        // Pass down the blockID to all the new children and unprune them
        for(int i = 0; i < 8; i++){
            subVoxels[i].parent = this;
            //subVoxels[i].blockID = blockID;
            subVoxels[i].level = level - 1;
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
    if(lvl == level){
        return this;
    }

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
    //return nullptr;
    return this;
}

// Function which determines if an arbitrary point in space is within this voxel
bool VoxelInstance::within(Vector3& position){
    // Calculate the "radius" of the cube based on its level
    int radius = pow(2, level);
    if(radius < 1) radius = 1;
    // Check if the arbitrary point is within those radius
    return !(position.x <= center.x - radius || position.x >= center.x + radius
        || position.y <= center.y - radius || position.y >= center.y + radius
        || position.z <= center.z - radius || position.z >= center.z + radius);
}

//Function which gets the visible faces from a voxel instance
void VoxelInstance::getFaces(std::vector<Face>& out){
    // Lambda which determines if the array contains a face
    auto notHas = [out](Face& what){
        for(const Face& f: out)
            if(f == what)
                return false;
        return true;
    };
    auto notHasR = [notHas](Face&& what){ return notHas(what); };
    // variable storing the "radius" of the voxel
    float bounds = pow(2, level - 1);
    if(checkInstanceFlags(Flags::TOP_VISIBLE)){
        Face f = Face(center + Vector3(-bounds, bounds, bounds),
            center + Vector3(bounds, bounds, bounds),
            center + Vector3(bounds, bounds, -bounds),
            center + Vector3(-bounds, bounds, -bounds), blockData->blockID).reverse();
        //if(notHas(f) && notHasR(f.reverse())) out.push_back(f);
        if(notHas(f)) out.push_back(f);
    }
    if(checkInstanceFlags(Flags::BOTTOM_VISIBLE)){
        Face f(center + Vector3(-bounds, -bounds, bounds),
            center + Vector3(bounds, -bounds, bounds),
            center + Vector3(bounds, -bounds, -bounds),
            center + Vector3(-bounds, -bounds, -bounds), blockData->blockID);
        //if(notHas(f) && notHasR(f.reverse())) out.push_back(f);
        if(notHas(f)) out.push_back(f);
    }
    if(checkInstanceFlags(Flags::NORTH_VISIBLE)){
        Face f(center + Vector3(bounds, -bounds, bounds),
            center + Vector3(bounds, bounds, bounds),
            center + Vector3(bounds, bounds, -bounds),
            center + Vector3(bounds, -bounds, -bounds), blockData->blockID);
        //if(notHas(f) && notHasR(f.reverse())) out.push_back(f);
        if(notHas(f)) out.push_back(f);
    }
    if(checkInstanceFlags(Flags::SOUTH_VISIBLE)){
        Face f = Face(center + Vector3(-bounds, -bounds, bounds),
            center + Vector3(-bounds, bounds, bounds),
            center + Vector3(-bounds, bounds, -bounds),
            center + Vector3(-bounds, -bounds, -bounds), blockData->blockID).reverse();
        //if(notHas(f) && notHasR(f.reverse())) out.push_back(f);
        if(notHas(f)) out.push_back(f);
    }
    if(checkInstanceFlags(Flags::EAST_VISIBLE)){
        Face f(center + Vector3(-bounds, bounds, bounds),
            center + Vector3(bounds, bounds, bounds),
            center + Vector3(bounds, -bounds, bounds),
            center + Vector3(-bounds, -bounds, bounds), blockData->blockID);
        //if(notHas(f) && notHasR(f.reverse())) out.push_back(f);
        if(notHas(f)) out.push_back(f);
    }
    if(checkInstanceFlags(Flags::WEST_VISIBLE)){
        Face f = Face(center + Vector3(-bounds, bounds, -bounds),
            center + Vector3(bounds, bounds, -bounds),
            center + Vector3(bounds, -bounds, -bounds),
            center + Vector3(-bounds, -bounds, -bounds), blockData->blockID).reverse();
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
    // TODO pass in ChunkMap pointer?
    // Distance to the center of the next voxel
    float distance = pow(2, level - 1) * 2;
    // Top
    if(find(level, center + Vector3(0, distance, 0))->blockData->checkFlags(BlockData::Flags::TRANSPARENT))
        flags |= VoxelInstance::Flags::TOP_VISIBLE;
    // Bottom
    if(find(level, center - Vector3(0, distance, 0))->blockData->checkFlags(BlockData::Flags::TRANSPARENT))
        flags |= VoxelInstance::Flags::BOTTOM_VISIBLE;
    // North
    if(find(level, center + Vector3(distance, 0, 0))->blockData->checkFlags(BlockData::Flags::TRANSPARENT))
        flags |= VoxelInstance::Flags::NORTH_VISIBLE;
    // South
    if(find(level, center - Vector3(distance, 0, 0))->blockData->checkFlags(BlockData::Flags::TRANSPARENT))
        flags |= VoxelInstance::Flags::SOUTH_VISIBLE;
    // East
    if(find(level, center + Vector3(0, 0, distance))->blockData->checkFlags(BlockData::Flags::TRANSPARENT))
        flags |= VoxelInstance::Flags::EAST_VISIBLE;
    // West
    if(find(level, center - Vector3(0, 0, distance))->blockData->checkFlags(BlockData::Flags::TRANSPARENT))
        flags |= VoxelInstance::Flags::WEST_VISIBLE;

    if(subVoxels)
        for(int i = 0; i < 8; i++)
            subVoxels[i].calculateVisibility();
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
        " - " + to_string(flags) + " - " + to_string(blockData->checkFlags(BlockData::Flags::TRANSPARENT))+ " - {" + center + "}\n";
    if(subVoxels)
        for(size_t i = 0; i < 8; i++)
            out += subVoxels[i].dump();
    return out;
}
