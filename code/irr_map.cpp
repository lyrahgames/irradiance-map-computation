namespace irr_map{


struct triangle_uvmap{
	uint order;
	vecf4 *data;

	vecf4 operator()(float u, float v) const;
};


inline uint size(uint order){
	return ((order+1)*(order+2))>>1;
}

inline uint memidx(uint uidx, uint vidx){
	const uint sum = uidx + vidx;
	return ((sum * (sum+1)) >> 1) + vidx;
}

inline uint memidx(uint uidx, uint vidx, uint shift){
	const uint suidx = uidx << shift;
	const uint svidx = vidx << shift;
	return memidx(suidx, svidx);
}

inline vec4f triangle_uvmap::operator()(float u, float v) const{
	const float nu = order * u;
	const float nv = order * v;
	const float uidx = floorf(nu);
	const float vidx = floorf(nv);
	const float wu = nu - uidx;
	const float wv = nv - vidx;
	const float sum_w = wu + wv;

	if (sum_w > 1.0f){
		const uint idx = memidx(uidx+1,vidx);
		const uint sum = uidx + vidx + 1;

		return (1.0f-wu) * data[idx + 1] + // memidx(u,v+1)
			(1.0f-wv) * data[idx] + // memidx(u+1,v)
			(sum_w-1.0f) * data[idx + sum + 2]; // memidx(u+1,v+1)
	}else{
		const uint idx = memidx(uidx, vidx);
		const uint sum = uidx + vidx;

		return wu * data[idx + sum + 1] + // memidx(u+1,v)
			wv * data[idx + sum + 2] + // memidx(u,v+1)
			(1.0f-sum_w) * data[idx]; // memidx(u,v)
	}
}


} // irr_map