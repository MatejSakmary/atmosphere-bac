#pragma once

#include <vulkan/vulkan.h>

/**
 * Create debug messenger to handle error messages from validation errors
 *  -> throws runtime error on failure
 */
void setupDebugMessenger(const VkInstance &instance, VkDebugUtilsMessengerEXT *debugMessenger);

/**
 * Populate Debug messenger create info with specified values
 * @param createInfo info to be populated
 */
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);