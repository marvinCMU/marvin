%classes =...
%{'car','dog','cat','bird','chair','pottedplant'};%'tvmonitor','bottle','cow'};
%classes = { 'motorbike' };

%cls = 'motorbike';
cls = 'train';
mode = 'e';

%% Here is the main script for doing the NN baseline

%Set default class and detector mode, by writing into the default
%file, which our mapreduces will be reading.
save_default_class(cls,mode);

%Run an "exemplar_initialize" mapreduce
timing.initialize = spawn_job('ei',50,2);

for q = 1:10
  %Load the models -- which will force a caching of result file
  models = load_all_models;
  
  timing.apply = spawn_job('ave2',80,2);
  res = load_result_grid(models);
  
  timing.train = spawn_job('do_train_mr',40,4);
  mode = [mode 'I'];
  save_default_class(cls,mode);
end


return
%Run an "apply_voc_exemplars"
timing.apply = spawn_job('ave2',80,2);

grid = load_result_grid(models);
clear M
M.betas = perform_calibration(models,grid);
[results,final] = evaluate_pascal_voc_grid(models, grid, 'test', M);
%[results,final] = evaluate_pascal_voc_grid(models, grid, 'test');

show_top_transfers(models,grid,'test',final)
