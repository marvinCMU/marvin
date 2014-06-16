% script for 3D labeling
function labeling3D
label_dir = 'label/';
points_3D = dlmread('structure.txt');
points_2D = dlmread('stitchedmeasurement_static.txt');
N3D  = size(points_3D,1);
for i = 1:N3D
    id3D = points_3D(i,1);
    correspondences = points_2D(id3D+1,:);
    class = corr_parser(correspondences,label_dir);
    ext_points_3D(i,:) = [points_3D(i,:) class];
    fprintf(['the ' num2str(i) 'th 3D point processed!\n']);
    if mod(i,2000) ==0
        save('ext_points_3D.mat','ext_points_3D');
    end
end
save('ext_points_3D.mat','ext_points_3D');


function class = corr_parser(correspondences,label_dir)
N = correspondences(1);
class = zeros(1,8);
cursor = 6;
for i = 1:N
    point = correspondences(cursor:cursor+5);
    imageID = point(2);
    y = point(5);
    x = point(6);
    keys = dlmread([label_dir 'image' gen_name(imageID+1) '.label']);
    dist = pdist2([x y], keys(:,1:2));
    [minval row] = min(dist);
%     row = find(x == keys(:,1) & y == keys(:,2));
    class = class + keys(row,3:10);
    cursor = cursor+6;
end



function name = gen_name(num)
name = num2str(num);
name = [repmat('0',[1 (7-length(name))]) name];

