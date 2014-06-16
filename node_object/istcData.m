addpath(genpath(pwd));
%% Dataset Params Like VOCinit (where to store etc.)

%devkitroot is where we write all the result files
%dataset_params.dataset = 'qolt_try_demo';
dataset_params.dataset = 'istc_try_demo';

%dataset_params.devkitroot = ['~/code/retailData/' ...
%    dataset_params.dataset];
dataset_params.devkitroot = ['retailData/' ...
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
