addpath(genpath(pwd));

vidExPipe;

models = {};
for i = obj_ind
   models = cat(2, models, load_all_models(dataset_params, cls{i}, ...
       [models_name '-svm'], [], 1, 1));
end