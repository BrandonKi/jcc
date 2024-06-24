#include "register_manager.h"

using namespace jb;


RegisterManager::RegisterManager() {

}

void RegisterManager::init() {
    used_gpr_set = {};
	free_gpr_set = gpr_mask;
	spilled_gpr_set = {};
}

Reg RegisterManager::alloc_gpr() {
/*
    if use_register_cache && cache != 0
	    take from cache and return
	else
*/
	auto selected_gpr = free_gpr_set.begin();
	auto selected_gpr_val = *selected_gpr;
	used_gpr_set.insert(selected_gpr_val);
	free_gpr_set.erase(selected_gpr);
    return (Reg)selected_gpr_val;
}

Reg RegisterManager::alloc_gpr(Reg gpr) {
    // TODO handle cache case

    if(!free_gpr_set.contains(gpr))
	    assert(false);
	else {
        free_gpr_set.erase(free_gpr_set.find(gpr));
		used_gpr_set.insert(gpr);
	}
	return gpr;
}

void RegisterManager::free_gpr(Reg gpr) {
    // TODO handle cache case

	if(!used_gpr_set.contains(gpr))
	    assert(false);
	else {
        used_gpr_set.erase(used_gpr_set.find(gpr));
		free_gpr_set.insert(gpr);
	}
}

void RegisterManager::spill_gpr(Reg gpr) {
    // TODO handle cache case

	if(!used_gpr_set.contains(gpr))
	    assert(false);
	else {
        used_gpr_set.erase(used_gpr_set.find(gpr));
		spilled_gpr_set.insert(gpr);
	}
}

/*void RegisterManager::foreach_spilled_gpr() {
    // TODO handle cache case

	if((gpr & used_gpr_set) == 0)
	    assert(false);
	else {
        used_gpr_set ^= gpr;
		spilled_gpr_set |= gpr;
	}
}*/
