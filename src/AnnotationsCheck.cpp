// AnnotationsChecker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <string>
#include <typeinfo>

namespace fs = std::filesystem;
//using namespace std;

int read_channelmap(std::string full_filepath, std::vector<std::string> &eeg_ch_labels);
int read_annotations(std::string full_filepath, std::vector<std::string> eeg_ch_labels, std::vector<std::string> &annots_ch_labels);

int main()
{
    std::string path("F:/Postdoc_Calgary/Research/Persyst_Project/EEG_Clips/");
    std::string ext(".lay");
    
    //std::cout << "Current path is " << fs::current_path() << '\n'; //
    path = fs::current_path().string() + "\\";


    // Loop through all files in specified folder
    for (auto& p : fs::recursive_directory_iterator(path)){

        std::string filename = p.path().stem().string();
        std::string file_extension = p.path().extension().string();

        std::vector<std::string> eeg_ch_labels;
        std::vector<std::string> annots_ch_labels;

        if (file_extension == ext) {
            std::cout << filename + file_extension << '\n';

            // Read .lay file
            std::string full_filepath = path + filename + file_extension;

            bool read_ok = false;

            read_ok = read_channelmap(full_filepath, eeg_ch_labels);

            read_ok = read_annotations(full_filepath, eeg_ch_labels, annots_ch_labels);

            std::cout <<  "\n\n\n";
        }
    }

    do {
        std::cout << '\n' << "Press Enter to continue...";
    } while (std::cin.get() != '\n');

    return 0;
}


int read_channelmap(std::string full_filepath, std::vector<std::string>& eeg_ch_labels) {

    // Create an input file stream object named 'file' and 
    std::ifstream file(full_filepath);

    // String to store each line of the file. 
    std::string line;

    bool start_channel_map = false;
    bool next_section_found = false;
    bool brackets_found = false;
    int line_nr = 0;

    if (file.is_open()) {
        // Read each line from the file and store it in the 'line' variable. 
        while (getline(file, line)) {
            line_nr++;

            //std::cout << line_nr << "\n";

            // Check start of channel map section
            if (line.compare("[ChannelMap]") == 0) {
                start_channel_map = true;
                //std::cout << "Start of ChannelMap" << '\n';
                continue;
            }

            if (start_channel_map) {

                // check that it is a channel entree
                if (line.find("-") != std::string::npos) {
                    std::string channel_label = line.substr(0, line.find('-'));

                    // to lowercase
                    for (int i = 0; i < channel_label.size(); i++) {
                        channel_label[i] = std::tolower(channel_label[i]);
                    }
                    
                    eeg_ch_labels.push_back(channel_label);
                    //std::cout << channel_label << "\t" << line << "\n";
                }
            }

            brackets_found = line.find("[") != std::string::npos && line.find("]") != std::string::npos;
            next_section_found = brackets_found & line.compare("[ChannelMap]") != 0;
            if (next_section_found && start_channel_map) {
                break;
            }
        }
        // Close the file stream.
        file.close();
    }
    else {
        // Print an error message to the standard error 
        // stream if the file cannot be opened. 
        std::cerr << "Unable to open file!" << std::endl;
        return false;
    }

    return true;
}

int read_annotations(std::string full_filepath, std::vector<std::string> eeg_ch_labels, std::vector<std::string>& incorrect_annots_ch_labels) {

    // Create an input file stream object named 'file' and 
    std::ifstream file(full_filepath);

    // String to store each line of the file. 
    std::string line;

    bool start_annotations= false;
    bool next_section_found = false;
    bool brackets_found = false;
    int line_nr = 0;
    int nr_annotations = 0;

    std::string annot_key = "@Spike";

    if (file.is_open()) {
        // Read each line from the file and store it in the 'line' variable. 
        while (getline(file, line)) {
            line_nr++;

            //std::cout << line_nr << "\n";

            // Check start of channel map section
            if (line.compare("[Comments]") == 0) {
                start_annotations = true;
                //std::cout << "Start of Comments" << '\n';
                continue;
            }

            if (start_annotations) {

                // check that it is a channel entree
                if (line.find(annot_key) != std::string::npos) {
                    
                    nr_annotations++;


                    int annot_pos = line.find("@Spike");
                    int reviewer_pos = line.find("r=");
                    int channel_pos = line.find("c=");
                    std::string reviewer = line.substr(reviewer_pos+2, channel_pos - reviewer_pos - 2);
                    reviewer.erase(remove_if(reviewer.begin(), reviewer.end(), isspace), reviewer.end());

                    std::string channel_label = line.substr(channel_pos+2, line.length()- channel_pos);
                    channel_label.erase(remove_if(channel_label.begin(), channel_label.end(), isspace), channel_label.end());
                    std::transform(channel_label.begin(), channel_label.end(), channel_label.begin(), [](unsigned char c) { return std::tolower(c); });

                    for (int i = 0; i < channel_label.size(); i++) {
                        channel_label[i] = std::tolower(channel_label[i]);
                    }

                    //std::cout << line << "\n";
                    //std::cout << reviewer << "\n";
                    //std::cout << channel_label << "\n";

                    std::string annot_time_sec = line.substr(0, line.find(','));
                    std::string annot_str = line.substr(line.find(annot_key), line.find("r="));
                    std::string reviewer_str = line.substr(line.find("r="), line.find("c="));
                    std::string ch_str = line.substr(line.find("c="), line.length() - 1);

                    float ann_total_seconds = std::stof(annot_time_sec);

                    int ann_hours = int(ann_total_seconds / 3600);
                    ann_total_seconds = ann_total_seconds - int(ann_hours * 3600);

                    int ann_minutes = int(ann_total_seconds / 60);
                    ann_total_seconds = ann_total_seconds - int(ann_minutes * 60);

                    int seconds = ann_total_seconds;


                    std::string annot_time_str = std::to_string(ann_hours) + ":" + std::to_string(ann_minutes) + ":" + std::to_string(seconds);
                    //std::cout << annot_time_str << "\n";

                    bool annot_chann_label_ok = false;
                    //std::cout << channel_label << "\n";
                    for (int i = 0; i < eeg_ch_labels.size(); i++) {
                        //std::cout << eeg_ch_labels[i] << "\n";
                        if (channel_label.compare(eeg_ch_labels[i]) == 0) {
                            annot_chann_label_ok = true;
                            break;
                        }

                    }

                    if (!annot_chann_label_ok) {
                        std::string wrong_annot = "@Spike r=" + reviewer + "c=" + ch_str + "\t\t" + annot_time_str;
                        incorrect_annots_ch_labels.push_back(wrong_annot);
                    }
                }
            }

            brackets_found = line.find("[") != std::string::npos && line.find("]") != std::string::npos;
            next_section_found = brackets_found & line.compare("[Comments]") != 0;
            if (next_section_found && start_annotations) {
                break;
            }
        }
        // Close the file stream.
        file.close();
    }
    else {
        // Print an error message to the standard error 
        // stream if the file cannot be opened. 
        std::cerr << "Unable to open file!" << std::endl;
        return false;
    }

    std::cout << "Nr. Annotations: " << nr_annotations << "\n";
    std::cout << "Nr. Incorrect Annotation Channel Labels: " << incorrect_annots_ch_labels.size() << "\n";

    for (int i = 0; i < incorrect_annots_ch_labels.size(); i++) {
        std::cout << incorrect_annots_ch_labels[i] << "\n";
    }

    return true;
}