for j,k in pairs(df.global.world.raws.mat_table.organic_types) do
	for i,mt in ipairs(df.global.world.raws.mat_table.organic_types[j]) do
		local mi = df.global.world.raws.mat_table.organic_indexes[j][i]

		local matinfo = dfhack.matinfo.decode(mt, mi)
		--print(matinfo, matinfo and matinfo.id)
		if matinfo and matinfo.material.state_color.Solid ~= -1 then
			matinfo.material.basic_color[0] = 100 + matinfo.material.state_color.Solid
			matinfo.material.basic_color[1] = 0

			matinfo.material.build_color[0] = 100 + matinfo.material.state_color.Solid
			--matinfo.material.build_color[1] = 100 + matinfo.material.state_color.Solid
			matinfo.material.build_color[2] = 0
		end
	end
end

for i,v in ipairs(df.global.world.raws.inorganics) do
	if v.material.state_color.Solid ~= -1 then
		v.material.basic_color[0] = 100 + v.material.state_color.Solid
		v.material.basic_color[1] = 0

		v.material.build_color[0] = 100 + v.material.state_color.Solid
		--v.material.build_color[1] = 100 + v.material.state_color.Solid
		v.material.build_color[2] = 0

		v.material.tile_color[0] = 100 + v.material.state_color.Solid
		--v.material.tile_color[1] = 100 + v.material.state_color.Solid
		v.material.tile_color[2] = 0
	end
end