% Load matrices O, C, CMsc, CMso, CMvo in xls format in BNxls_folder
% into mat file stored in BNmat_folder
function BNxls2mat() 

BNxls_folder = '~/code/code_data/confusion_matrix/';
BNmat_folder = '~/code/code_data/';

O = xlsread([BNxls_folder 'O.xls']);
C = xlsread([BNxls_folder 'C.xls']);
CMsc = xlsread([BNxls_folder 'Sc_CM.xls']);
CMso = xlsread([BNxls_folder 'So_CM.xls']);
CMvo = xlsread([BNxls_folder 'Vo_CM.xls']);

save([BNmat_folder 'confusion_matrix'], 'O', 'C', 'CMsc', 'CMso', 'CMvo');