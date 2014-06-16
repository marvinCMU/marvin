addpath(genpath(pwd));
display('Object detector initializing!');
% initialization


% parameters
pwd = pwd();
data_path = [pwd(1:end-11),'data_tmp/']

vision_folder = fullfile(data_path,'vision');
vision_processed_folder = fullfile(data_path,'vision_processed');
model_filename = 'new_models/bookstore_models.mat';
mapping_filename = 'new_models/mapping.mat';
save_filename = 'runningtime2.mat';
sleeping_time = 0.2;

params = esvm_get_default_params;
max_min_ImgSize_seq = [275 325 350]; % the maximum pixel number for smaller dimension
N_model_seq = [100 200 400 600 1000 1500 2000 2232];
cross_exemplar_nms_thresh = 0.3;
thresh = 0.9;

load(model_filename);
load(mapping_filename);
params.ranked_list_filename = [];
params.displayRankedList = 1;
params.truncatedModels = 0;
params.GPUacc = 1;
display('Initilizing done, start scanning...');
img_list = dir(fullfile(vision_folder,'*.jpg'));
img_list = arrayfun(@(x)x.name,img_list,'UniformOutput',false);
N = 20;

record = [];
for size_idx = 1:length(max_min_ImgSize_seq)
    
    max_min_ImgSize = max_min_ImgSize_seq(size_idx);
    for N_model_idx = 1:length(N_model_seq)
        N_model = N_model_seq(N_model_idx);
        submodels = models(1:N_model);
        params.exemplar_matrix = gen_exemplar_matrix(submodels,params);
        for i = 1:N
            img_filename = fullfile(vision_folder,img_list{i});
            I = imread(img_filename);

            % reduce image size to speed up
            if min(size(I,1),size(I,2)) > max_min_ImgSize
                ratio = max_min_ImgSize/min(size(I,1),size(I,2));
                I = imresize(I,ratio);
            end

            params.precomputation = [];
            starttime = tic;
            [rs,t] = esvm_detect_fast(I, submodels, params);
            timegap = toc(starttime);
            record = [record;max_min_ImgSize N_model timegap];
            fprintf(['size: ' num2str(max_min_ImgSize) ', number of models: ' num2str(N_model)...
                ', time: ' num2str(timegap) '\n']);

        end
        save(save_filename,'record');
    end
end
