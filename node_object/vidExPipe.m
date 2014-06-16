% addpath(genpath(pwd));
%% Dataset Params Like VOCinit (where to store etc.)

%devkitroot is where we write all the result files
%dataset_params.dataset = 'qolt_try_demo';
dataset_params.dataset = 'istc_try_demo';

dataset_params.devkitroot = ['~/code/retailData/' ...
    dataset_params.dataset];

dataset_params.wwwdir = [dataset_params.devkitroot '/www/'];

% change this path to a writable local directory for the example code
dataset_params.localdir=[dataset_params.devkitroot '/local/'];

% change this path to a writable directory for your results
dataset_params.resdir=[dataset_params.devkitroot ['/' ...
                    'results/']];
dataset_params.display = 0;
%dataset_params.testset = 'test';
%dataset_params = VOCinit(dataset_params);

%% Name Parameters (What to call the models?)
scenestring = 'exemplar';

%Choose a short string to indicate the type of training run we are doing
models_name = ['g.' scenestring];
% return

%% How to initialize Exemplars?

%Choose Fixed-Frame initialization function, and its parameters
%init_function = @initialize_fixedframe_model;
%init_params.sbin = 8;
%init_params.hg_size = [8 8];

%Choose Goal-Cells initialize function, and its parameters
init_params.goal_ncells = 100;
init_params.MAXDIM = 10;
init_params.sbin = 8;
init_params.init_type = 'goalsize';
init_params.init_function = @initialize_goalsize_model;


%% Default Mining Parameters?

% get_default_param_f = @get_default_mining_params;
default_mining_f = @get_default_mining_params_video;
training_function = @do_svm;
%Get the default mining parameters
mining_params = default_mining_f();

%%
load(dataset_params.dataset, 'cls');
% return here when not training
return;

%% Setup Streams?
stream_f = @get_exemplar_stream_folder;

% folder = ['/nfs/ladoga_no_backups/users/ashrivas/datasets/qoltKitchen/images/'];
folder = ['/nfs/ladoga_no_backups/users/ashrivas/datasets/istcData/positiveImages'];
%No of objects to select per frame..
N_Frame = 1;
e_stream_set = stream_f(dataset_params, folder, N_Frame);

% % get the negative set for training
% neg_path = '/nfs/onega_no_backups/users/ashrivas/datasets/gDemo/negative/';
% train_set = get_video_bg(neg_path);
% % make length atleast 5000
% if length(train_set)<10000
%     bg = get_james_bg(10000-length(train_set));
%     train_set = cat(2, train_set, bg');
% end
neg_path = '/nfs/ladoga_no_backups/users/ashrivas/datasets/istcData/negativeImages/';
neg_names = arrayfun(@(x) x.name, dir([neg_path '*.jpg']), 'UniformOutput', false); 
neg_set = cellfun(@(x) [neg_path x], neg_names, 'UniformOutput', false);

%% Find cls!
clss = cellfun(@(x) x.cls, e_stream_set, 'UniformOutput',false);
clssU = unique(clss);
e_stream_cls = cell(0, length(clssU));
for cl = 1:length(clssU)
    e_stream_cls{cl} = e_stream_set(cell2mat(cellfun(@(x)...
        strcmp(x.cls, clssU{cl}),  e_stream_set, 'UniformOutput',false)));
end
cls = clssU;
%save(dataset_params.dataset, 'cls');
%% RUN Actual Traning....

% for i=1:length(cls)
%     efiles = exemplar_initialize(dataset_params, e_stream_cls{i}, ...
%         models_name, init_params);
% end
% wait_until_all_present(efiles, 10);
% 
% % return;
% 
% for i=1:length(cls)
%     models = load_all_models(dataset_params, cls{i},models_name,{},1);
%     train_all_exemplars(dataset_params, models, neg_set, mining_params, ...
%         training_function);
% end
for i=1:length(cls)
    efiles = exemplar_initialize(dataset_params, e_stream_cls{i}, ...
        models_name, init_params);
    models = load_all_models(dataset_params, cls{i},models_name,efiles,1);
    
    train_all_exemplars(dataset_params, models, neg_set, mining_params, ...
        training_function);
end
