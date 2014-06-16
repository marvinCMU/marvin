addpath(genpath(pwd));

%%
vidExPipe;
%%
% a 1    'appliances'
% b 2    'boxes_translucent'
% c 3    'can_green'
% d 4    'can_white'
% e 5   'coffee_maker'
% f 6    'cutlery_box'
% g 7    'oil'
% h 8    'salt_box'
% i 9    'salt_can'
% j 10    'salt_whitebox'
% k 11    'spray_can'
% l 12    'spray_cleaner'
% m 13    'wipes'
%%
models = {};
for i = [2 4 5 10 12 13]
    models = cat(2, models, load_all_models(dataset_params, cls{i}, ...
        [models_name '-svm'], [], 1, 1));
end