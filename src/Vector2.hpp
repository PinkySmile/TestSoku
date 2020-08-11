//
// Created by Gegel85 on 04/06/2020.
//

#ifndef SOKUTEST_VECTOR2_HPP
#define SOKUTEST_VECTOR2_HPP


namespace Soku
{
	template<typename type>
	struct Vector2 {
		type x;
		type y;
	};

	typedef Vector2<int> Vector2i;
	typedef Vector2<unsigned> Vector2u;
	typedef Vector2<double> Vector2d;
	typedef Vector2<float> Vector2f;
}


#endif //SOKUTEST_VECTOR2_HPP
