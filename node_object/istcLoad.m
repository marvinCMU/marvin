load istc_try_demo.mat

addpath(genpath(pwd));

istcData;

models = {};
obj_ind = 1:12;
for i = obj_ind
   models = cat(2, models, load_all_models(dataset_params, cls{i}, ...
       [models_name '-svm'], [], 1, 1));
end