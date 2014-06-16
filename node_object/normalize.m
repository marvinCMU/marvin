% Input:
% M = Matrix to be normalize over row or col (see which below) 
% which = 
%   1: normalize each column
%   2: normalize each row
%
% Output:
% normalized_M = Matrix M in normalized form 
function normalized_M = normalize(M, which)

normalized_M = bsxfun(@times, M, 1./sum(M, which));