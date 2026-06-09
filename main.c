/*
Roam - a nostalgic way to wander around the world

Copyright (c) 2026 Benjamin Chase (GitHub @perspector) MatrikelNr 317190, Kamer Burak Isci (GitHub @kamerburak) MatrikelNr 317470

Based on Mapillary's API using crowdsourced image data (https://mapillary.com)
Also uses Nominatim's API for geocoding (https://nominatim.org)
Special thanks to Xander Gouws (GitHub @gouwsxander) for his incredible ascii-viewer! (https://github.com/gouwsxander/ascii-view)
*/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include "cJSON.h"

// for ASCII art, from https://github.com/gouwsxander/ascii-view
#include "ascii-view/include/image.h"
#include "ascii-view/include/print_image.h"


// struct to store data returned from Mapillary API GET requests
struct Memory {
    char *response;
    size_t size;
};


// struct to store image data as bytes which will be written to a .jpg file
struct JPGFileData {
    char *bytes;
    size_t size;
};


// used to transfer the array of image IDs between functions, adapted from https://www.geeksforgeeks.org/c/return-an-array-in-c/
struct ImageIDs {
    char array[1000][24]; // TODO: change 1000 to max number of images in a sequence? Or use a dynamic array?
};


// used to transfer image metadata between functions
struct ImageMetadata {
    char id[24]; // TODO: reduce length?
    char camera_type[100]; // TODO: reduce length!
    long int captured_at;
    double compass_angle;
    double computed_compass_angle;
    char creator[100];
    int height;
    int is_pano;
    char thumb_original_url[1024];
    float quality_score;
    char sequence[50];
    int width;
    char filename[500]; // TODO: reduce length!
};


// Resets the buffering of the terminal to be default (unbuffered) so text is shown immediately.
void reset_buffering() {
    setbuf(stdout, NULL);
}


// callback function for curl, writes response data from API to memory, adapted from https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html and OpenAI ChatGPT (GPT-5.5)
size_t write_callback(void *data, size_t size, size_t nmemb, void *clientp)
{
    size_t total_size = size * nmemb;
    struct Memory *mem = (struct Memory *)clientp;
    char *ptr = realloc(mem->response, mem->size + total_size + 1);

    if (ptr == NULL) {
        reset_buffering();
        printf("Out of memory\n");
        return 0;
    }

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, total_size);
    mem->size += total_size;
    mem->response[mem->size] = '\0';
    return total_size;
}


// callback function for curl, write data to file, adapted from https://stackoverflow.com/a/1636415
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}


// Gets image metadata using Mapillary's API.
// `image_id` individual unique image ID from Mapillary
// `key` Mapillary API key
// Returns an ImageMetadata struct containing data including camera type, time captured, is it panoramic, etc.
struct ImageMetadata get_image_metadata(char *image_id, char *key) {
    // make another API GET request to get the actual image data using the ID

    // initialize memory where the response can be stored
    struct Memory chunk;
    chunk.response = malloc(1);
    chunk.size = 0;

    struct ImageMetadata image_metadata = {"", "", 0, 0, 0, "", 0, 0, "", 0, "", 0, ""};

    // format the request URL
    char request_url[350]; // TODO: reduce 512, I do not need this much memory
    snprintf(request_url, 350, "https://graph.mapillary.com/%s?access_token=%s&fields=camera_type,captured_at,compass_angle,computed_compass_angle,creator,height,is_pano,thumb_original_url,quality_score,sequence,width", image_id, key);

    CURL *curl = curl_easy_init();
    if (curl) {
        CURLcode result;
        // set curl options
        curl_easy_setopt(curl, CURLOPT_URL, request_url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &chunk);
        result = curl_easy_perform(curl);
        if (result != CURLE_OK) {
            // errors in GET request
            reset_buffering();
            fprintf(stderr, "curl error: %s\n", curl_easy_strerror(result));
        } else {
            // successful GET request
            // parse response as JSON using cJSON to get the image properties
            cJSON *root = cJSON_Parse(chunk.response);

            //printf("Mapillary API image data output:\n%s\n", cJSON_Print(root));

            // store image properties as variables
            if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(root, "camera_type")) && cJSON_GetObjectItemCaseSensitive(root, "camera_type") -> valuestring != NULL) {
                strcpy(image_metadata.camera_type, cJSON_GetObjectItemCaseSensitive(root, "camera_type") -> valuestring);
            } else {
                strcpy(image_metadata.camera_type, "");
            }
            if (cJSON_IsNumber(cJSON_GetObjectItemCaseSensitive(root, "captured_at"))) {
                image_metadata.captured_at = cJSON_GetObjectItemCaseSensitive(root, "captured_at") -> valuedouble;
            } else {
                image_metadata.captured_at = 0;
            }
            if (cJSON_IsNumber(cJSON_GetObjectItemCaseSensitive(root, "compass_angle"))) {
                image_metadata.compass_angle = cJSON_GetObjectItemCaseSensitive(root, "compass_angle") -> valuedouble;
            } else {
                image_metadata.compass_angle = 0;
            }
            if (cJSON_IsNumber(cJSON_GetObjectItemCaseSensitive(root, "computed_compass_angle"))) {
                image_metadata.computed_compass_angle = cJSON_GetObjectItemCaseSensitive(root, "computed_compass_angle") -> valuedouble;
            } else {
                image_metadata.computed_compass_angle = 0;
            }

            cJSON *creator = cJSON_GetObjectItemCaseSensitive(root, "creator");
            strcpy(image_metadata.creator, cJSON_GetObjectItemCaseSensitive(creator, "username") -> valuestring);

            if (cJSON_IsNumber(cJSON_GetObjectItemCaseSensitive(root, "height"))) {
                image_metadata.height = cJSON_GetObjectItemCaseSensitive(root, "height") -> valueint;
            } else {
                image_metadata.height = 0;
            }

            if (cJSON_IsBool(cJSON_GetObjectItemCaseSensitive(root, "is_pano"))) {
                image_metadata.is_pano = cJSON_GetObjectItemCaseSensitive(root, "is_pano") -> type == cJSON_True ? 1 : 0;
            } else {
                image_metadata.is_pano = 0;
            }

            if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(root, "thumb_original_url")) && cJSON_GetObjectItemCaseSensitive(root, "thumb_original_url") -> valuestring != NULL) {
                strcpy(image_metadata.thumb_original_url, cJSON_GetObjectItemCaseSensitive(root, "thumb_original_url") -> valuestring);
            } else {
                strcpy(image_metadata.thumb_original_url, "");
            }
            if (cJSON_IsNumber(cJSON_GetObjectItemCaseSensitive(root, "quality_score"))) {
                image_metadata.quality_score = cJSON_GetObjectItemCaseSensitive(root, "quality_score") -> valuedouble;
            } else {
                image_metadata.quality_score = 0;
            }
            if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(root, "sequence")) && cJSON_GetObjectItemCaseSensitive(root, "sequence") -> valuestring != NULL) {
                strcpy(image_metadata.sequence, cJSON_GetObjectItemCaseSensitive(root, "sequence") -> valuestring);
            } else {
                strcpy(image_metadata.sequence, "");
            }
            if (cJSON_IsNumber(cJSON_GetObjectItemCaseSensitive(root, "width"))) {
                image_metadata.width = cJSON_GetObjectItemCaseSensitive(root, "width") -> valueint;
            } else {
                image_metadata.width = 0;
            }
            //printf("%s\n", image_metadata.camera_type);
            //printf("%li\n", image_metadata.captured_at);
            //printf("%lf\n", image_metadata.compass_angle);
            //printf("%lf\n", image_metadata.computed_compass_angle);
            //printf("%s\n", image_metadata.creator);
            //printf("%i\n", image_metadata.height);
            //printf("%i\n", image_metadata.is_pano);
            //printf("%s\n", image_metadata.thumb_original_url);
            //printf("%f\n", image_metadata.quality_score);
            //printf("%s\n", image_metadata.sequence);
            //printf("%i\n", image_metadata.width);

            cJSON_Delete(root);
            curl_easy_cleanup(curl);
            //printf("> Saved metadata for image %s\n", image_id);

            return image_metadata;
        }
    }
    curl_easy_cleanup(curl);
    free(chunk.response); // free memory
    return image_metadata;
}


// Gets the ID of a street view image using Mapillary's API.
// `lat` latitude (-90 to 90)
// `lng` longitude (-180 to 180)
// `allow_pano` allow panoramic images? 0 = true, 1 = false
// `key` Mapillary API key
// Returns an array containing image IDs as strings using an ImageIDs struct.
// The array can be accessed using get_image_ids_at_coords(...).array
// 
// If an image is not part of a sequence, the array will just contain one ID.
// If the image is part of the sequence, all image IDs of the images in the sequence will be returned.
struct ImageIDs get_image_ids_at_coords(double lat, double lng, int allow_pano, char *key) {
    //printf("Searching for images at coordinates...\n");

    int radius = 50; // radius in meters to search around coordinates, TODO: set dynamically, check for images close by and then further OR I think Mapillary already does this for me :) max 50
    int limit = 100; // number of photo IDs to get, TODO: experiment with different limits and see impact on UX, max 100

    // get image ID of closest image at given coordinates

    struct ImageIDs image_ids;

    // make API GET request
    // uses code from https://curl.se/libcurl/c/curl_easy_init.html

    // initialize memory where the response can be stored
    struct Memory chunk;
    chunk.response = malloc(1);
    chunk.size = 0;

    // format the request URL
    char request_url[200]; // TODO: reduce 512, I do not need this much memory
    snprintf(request_url, 200, "https://graph.mapillary.com/images?fields=id&lat=%lf&lng=%lf&radius=%i&limit=%i&access_token=%s", lat, lng, radius, limit, key);

    // make the request using curl
    CURL *curl = curl_easy_init();
    if (curl) {
        CURLcode result;
        // set curl options
        curl_easy_setopt(curl, CURLOPT_URL, request_url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &chunk);
        result = curl_easy_perform(curl);
        if (result != CURLE_OK) {
            // errors in GET request
            reset_buffering();
            fprintf(stderr, "curl error: %s\n", curl_easy_strerror(result));
            strcpy(image_ids.array[0], "");
            return image_ids;
        } else {
            // successful GET request
            // parse response as JSON using cJSON to get the image ID
            cJSON *root = cJSON_Parse(chunk.response);
            //printf("Mapillary API image ID output:\n%s\n", cJSON_Print(root));

            cJSON *image;
            cJSON *images = cJSON_GetObjectItemCaseSensitive(root, "data");
            int i = 0;
            cJSON_ArrayForEach(image, images) {
                i++;
            }

            i = 0;
            cJSON_ArrayForEach(image, images) {
                //printf("> Found image ID at coordinates: %s\n", cJSON_GetObjectItemCaseSensitive(image, "id") -> valuestring);
                strcpy(image_ids.array[i], cJSON_GetObjectItemCaseSensitive(image, "id") -> valuestring);
                i++;
            }
            cJSON_Delete(root);
            curl_easy_cleanup(curl);
            free(chunk.response);

            //printf("> Checking whether images are in sequences\n");

            // Now that we have the image IDs from Mapillary around the specified coordinates in an array, we can check whether each image ID if it is in a sequence.
            // If it is in a sequence, we return all IDs in the sequence.
            char *first_sequence_id = "";
            for (int i = 0; i < sizeof(image_ids.array) / sizeof(image_ids.array[0]); i++) {
                // for each image ID, check if it is in a sequence
                struct ImageMetadata image_metadata = get_image_metadata(image_ids.array[i], key);
                // does not have empty sequence               and      first sequence not set, so == ""  or       first sequence == image sequence
                if (strcmp(image_metadata.sequence, "") && (!strcmp(first_sequence_id, "") || !strcmp(first_sequence_id, image_metadata.sequence))) {
                    // image is in a sequence

                    // check if we allow panoramic images OR image is not panoramic
                    //printf("%i || !%i\n", allow_pano, image_metadata.is_pano);
                    if (allow_pano || (!image_metadata.is_pano)) {
                        //printf("> Image is in sequence %s\n> Searching for other images in the sequence...\n", image_metadata.sequence);
                        first_sequence_id = image_metadata.sequence;
                        // empty image_ids.array
                        for (int i; i < sizeof(image_ids.array) / sizeof(image_ids.array[0]); i++) {
                            strcpy(image_ids.array[i], "");
                        }
                        // do a query using https://www.mapillary.com/developer/api-documentation#sequence as a reference
                        // make API GET request
                        // uses code from https://curl.se/libcurl/c/curl_easy_init.html

                        // initialize memory where the response can be stored
                        struct Memory chunk;
                        chunk.response = malloc(1);
                        chunk.size = 0;

                        // format the request URL
                        char request_url[200]; // TODO: reduce 512, I do not need this much memory
                        snprintf(request_url, 200, "https://graph.mapillary.com/image_ids?sequence_id=%s&access_token=%s", image_metadata.sequence, key);

                        // make the request using curl
                        CURL *curl = curl_easy_init();
                        if (curl) {
                            CURLcode result;
                            // set curl options
                            curl_easy_setopt(curl, CURLOPT_URL, request_url);
                            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
                            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &chunk);
                            result = curl_easy_perform(curl);
                            if (result != CURLE_OK) {
                                // errors in GET request
                                reset_buffering();
                                fprintf(stderr, "curl error: %s\n", curl_easy_strerror(result));
                                strcpy(image_ids.array[0], "");
                                return image_ids;
                            } else {
                                // successful GET request
                                // parse response as JSON using cJSON to get the image ID
                                cJSON *root = cJSON_Parse(chunk.response);
                                //printf("Mapillary API images in sequence:\n%s\n", cJSON_Print(root));

                                cJSON *image;
                                cJSON *images = cJSON_GetObjectItemCaseSensitive(root, "data");
                                int i = 0;
                                cJSON_ArrayForEach(image, images) {
                                    i++;
                                }

                                i = 0;
                                cJSON_ArrayForEach(image, images) {
                                    //printf("    > Found image ID in sequence: %s\n", cJSON_GetObjectItemCaseSensitive(image, "id") -> valuestring);
                                    strcpy(image_ids.array[i], cJSON_GetObjectItemCaseSensitive(image, "id") -> valuestring);
                                    i++;
                                }
                                cJSON_Delete(root);
                                curl_easy_cleanup(curl);
                                free(chunk.response);

                                return image_ids;
                            }
                        }
                    }
                }
            }

            // if no sequence ID was found, delete everything in the image_ids.array array except for the first image ID
            //printf("No sequence of images was found, returning first image only\n");
            for (int i; i < sizeof(image_ids.array) / sizeof(image_ids.array[0]); i++) {
                if (i != 0) {
                    strcpy(image_ids.array[i], "");
                }
            }
            return image_ids;
        }
    }
    strcpy(image_ids.array[0], "");
    return image_ids;
}


// Saves a single image as .jpg using Mapillary's API.
// `image_id` individual unique image ID from Mapillary
// `hdg` heading (0 to 360) (only available for panoramas)
// `key` Mapillary API key
// Returns the ImageMetadata struct containing details about the image
struct ImageMetadata save_image(char *image_id, int order_in_sequence, double hdg, char *key) {
    //char thumb_original_url[1024]; // TODO: reduce size
    struct ImageMetadata image_metadata = get_image_metadata(image_id, key);
    //strcpy(thumb_original_url, image_metadata.thumb_original_url);
    strcpy(image_metadata.id, image_id);

    // make another GET request, this time to thumb_original_url to get the image pixel data

    //printf("Downloading image from %s\n", thumb_original_url);

    // initialize memory where the response can be stored
    struct JPGFileData file_chunk;
    file_chunk.bytes = malloc(1);
    file_chunk.size = 0;

    // format the request URL
    char request_url[512]; // TODO: reduce 512, I do not need this much memory
    snprintf(request_url, 512, "%s", image_metadata.thumb_original_url);

    CURL *curl = curl_easy_init();
    if (curl) {
        //static char image_filename[150]; // TODO: reduce 150, I do not need this much memory
        snprintf(image_metadata.filename, sizeof(image_metadata.filename), "image_cache/%s_%04i_%s_%s.jpg", image_metadata.sequence, order_in_sequence, image_id, image_metadata.creator); // format <sequence>_<num in sequence>_<image id>.jpg
        FILE *image_file = fopen(image_metadata.filename, "wb"); // file to temporarily store image to
        CURLcode result;
        // set curl options
        curl_easy_setopt(curl, CURLOPT_URL, request_url);
        // following options for downloading to a file adapted from https://stackoverflow.com/a/1636415
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, image_file);
        result = curl_easy_perform(curl);
        if (result != CURLE_OK) {
            // errors in GET request
            reset_buffering();
            fprintf(stderr, "curl error: %s\n", curl_easy_strerror(result));
        } else {
            // successfully downloaded file
            //printf("> Saved image to %s\n", image_filename);
        }
        curl_easy_cleanup(curl);
        fclose(image_file);
    }
    free(file_chunk.bytes); // free memory
    return image_metadata;
}

// Clears the image cache folder.
void delete_images() {
    remove("image_cache/*.jpg");
    // TODO: finish this!!!
}

// Converts human-readible address to latitude and longitude coordinates using Nominatim's public API.
// `address` address to get coordinates for
// Returns an array of length 2 consisting of doubles in the form {<latitude>, <longitude>}.
double* address_to_coords(char *address) {
    static double coords[2] = {0};

    // initialize memory where the response can be stored
    struct Memory chunk;
    chunk.response = malloc(1);
    chunk.size = 0;

    // format the request URL
    char request_url[200]; // TODO: reduce 512, I do not need this much memory
    snprintf(request_url, 200, "https://nominatim.openstreetmap.org/search?q=%s&format=jsonv2", address);
    // replace spaces in address with +
    for (int i=0; request_url[i] != '\0'; i++) {
        if (request_url[i] == ' ') {
            request_url[i] = '+';
        }
    }
    printf("Geocoding coordinates from address using Nominatim: %s\n", request_url);

    // make the request using curl
    CURL *curl = curl_easy_init();
    if (curl) {
        CURLcode result;
        // set curl options
        curl_easy_setopt(curl, CURLOPT_URL, request_url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &chunk);
        // Nominatim requires a User Agent or referrer URL, see https://operations.osmfoundation.org/policies/nominatim/
        // This was helpful for setting the referrer: https://everything.curl.dev/libcurl-http/requests.html
        curl_easy_setopt(curl, CURLOPT_REFERER, "https://github.com/perspector/roam/");
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:150.0) Gecko/20100101 Firefox/150.0");
        result = curl_easy_perform(curl);
        if (result != CURLE_OK) {
            // errors in GET request
            reset_buffering();
            fprintf(stderr, "curl error: %s\n", curl_easy_strerror(result));
            return coords;
        } else {
            // successful GET request
            // parse response as JSON using cJSON to get the image ID
            cJSON *root = cJSON_Parse(chunk.response);
            //printf("Nominatim geocoder API image ID output:\n%s\n", cJSON_Print(root));

            cJSON *place;
            int i = 0;
            char *stop_str;
            cJSON_ArrayForEach(place, root) {
                if (i == 0) {
                    coords[0] = strtod(cJSON_GetObjectItemCaseSensitive(place, "lat") -> valuestring, &stop_str);
                    coords[1] = strtod(cJSON_GetObjectItemCaseSensitive(place, "lon") -> valuestring, &stop_str);
                }
                i++;
            }

            cJSON_Delete(root);
            curl_easy_cleanup(curl);
            free(chunk.response);
            return coords;
        }
    }
    return coords;
}


// Front-end for displaying images
// Uses code from https://github.com/gouwsxander/ascii-view for displaying images as ASCII
int display_image(struct ImageMetadata image_metadata, int max_width, int max_height, float character_ratio, float edge_threshold, int use_retro_colors) {
    // TODO: Future improvements:
    // see https://help.mapillary.com/hc/en-us/articles/115001465989-360-cameras
    // and see https://ikyle.me/blog/2026/render-360-equirectangular
    // and also see https://developers.google.com/streetview/spherical-metadata
    // REMEMBER TO CREDIT MAPILLARY USING THEIR LOGO IF POSSIBLE (use `kitty` graphics?)
    // AND CREDIT USERS: "Title" <Link to Mapillary image> by “username” <Link to user profile>, licensed under CC-BY-SA.
    // for example: 1749783712282209 (https://www.mapillary.com/app/?pKey=1749783712282209) by RadNETZ (https://www.mapillary.com/app/user/RadNETZ), licensed under CC-BY-SA

    // Loads image
    image_t original = load_image(image_metadata.filename);
    if (!original.data)
        return 1;

    // Resizes image
    image_t resized = make_resized(&original, max_width, max_height, character_ratio);
    if (!resized.data) {
        free_image(&original);
        return 1;
    }
    
    print_image(&resized, edge_threshold, use_retro_colors);
    
    free_image(&original);
    free_image(&resized);

    printf("ID: %s (https://www.mapillary.com/app/?pKey=%s) by %s (https://www.mapillary.com/app/user/%s), licensed under CC-BY-SA\n", image_metadata.id, image_metadata.id, image_metadata.creator, image_metadata.creator);

    return 0;
}


// main program
int main() {
    // clear image cache folder
    delete_images();

    // opening sequence, greet user
    printf("\n\nRoam\n\n\n");
    printf("Street-level imagery powered by Mapillary (https://mapillary.com)\nGeocoding powered by Nominatim (https://nominatim.org)\n");
    printf("All imagery is licensed under the Creative Commons Share Alike (CC BY-SA) license, unless stated otherwise.\nMore information is available at https://mapillary.com/terms\n\n\n");

    double *coords = address_to_coords("Fahrradbruecke Konstanz");
    printf("Geocoded coordinates using Nominatim: lat: %lf lon: %lf\n", coords[0], coords[1]);

    char *key = getenv("MAPILLARY_TOKEN");

    struct ImageIDs image_ids;
    printf("Searching for a sequence of images near the coordinates...\n");
    image_ids = get_image_ids_at_coords(coords[0], coords[1], 0, key);
    if (strcmp(image_ids.array[0], "")) {
        int num_images = 0;
        for (int i = 0; i < sizeof(image_ids.array) / sizeof(image_ids.array[0]); i++) {
            // if the image ID is present in the array and is not an empty space
            if (strcmp(image_ids.array[i], "")) {
                num_images++;
            }
        }
        printf("Found images at coordinates!\nSaving images... 0001 / %04i", num_images);
        fflush(stdout); // since we're not printing a newline, most terminals require manually flushing content buffer to screen

        // Make the terminal fully buffered, so that each frame can be printed at once. Reduces frame flickering.
        setvbuf(stdout, NULL, _IOFBF, 4096);

        for (int i = 0; i < num_images; i++) {
            // if the image ID is present in the array and is not an empty space
            if (strcmp(image_ids.array[i], "")) {
                //printf("\b\b\b\b\b\b\b\b\b\b\b%04i / %04i", i + 1, num_images);
                //fflush(stdout); // since we're not printing a newline, most terminals require manually flushing content buffer to screen
                struct ImageMetadata image_metadata;
                image_metadata = save_image(image_ids.array[i], i, 0.0, key);
                
                printf("\n\n\n\n\n\n\n\n\n\n");
                //fflush(stdout);
                display_image(image_metadata, 200, 68, 2, 1.5, 0);

                printf("\nImage %04i / %04i | All images are from Mapillary (https://mapillary.com)\n", i + 1, num_images);
                fflush(stdout);
            }
        }

        // reset buffering to no buffering so output is shown immediately as before
        setbuf(stdout, NULL);

        printf("\n");
    } else {
        printf("\033[31m \n[!] No image could be found at the specified location, please try another location.\nMapillary is crowdsourced by people like you! You can help by adding some imagery at [https://mapillary.com]\033[0m\n\n");
    }

    return 0;
}
