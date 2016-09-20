local AOI=require 'laoi.c'

local MEMSIZE={
	{"T",1024*1024*1024*1024},
	{"G",1024*1024*1024},
	{"M",1024*1024},
	{"K",1024},
	{"B",0},
}

local function MEM(s)
	for _,c in pairs(MEMSIZE) do
		local n,m=c[1],c[2]
		if s>=m then
			return string.format("%.02f%s",s/m,n)
		end
	end
end

local aoi=AOI.new(5,function(id,intb,outtb,movetb)
	if intb then
		for _,i in ipairs(intb) do
			--print(id .." in ".. i)
		end
	end
	if outtb then
		for _,i in ipairs(outtb) do
			--print(id .." out ".. i)
		end
	end
	if movetb then
		for _,i in ipairs(movetb) do
			--print(id .." move ".. i)
		end
	end
end)

local m={
{1	,7	,8	,0	,0	},
{2	,1	,8	,0	,0	},
{3	,1	,10	,0	,0	},
{4	,8	,8	,0	,0	},
{5	,5	,7	,0	,0	},
{6	,3	,3	,0	,0	},
{7	,5	,8	,0	,0	},
{8	,10	,2	,0	,0	},
{9	,0	,7	,0	,0	},
{10	,8	,0	,0	,0	},
}

local max=10000
local maxtime=10000
print(MEM(AOI.memory()),os.clock())
for i=1,maxtime do
	AOI.add(aoi,i,math.random(1,max+1)-1,math.random(1,max+1)-1)
end
print(MEM(AOI.memory()),os.clock())
for i=1,maxtime do
	AOI.move(aoi,i,math.random(1,max+1)-1,math.random(1,max+1)-1)
end
print(MEM(AOI.memory()),os.clock())