TLVLength = (function() {
	function TLVLength() {
		if (arguments.length) {
			if (jQuery.isArray(arguments[0]))
				this.binary = arguments[0];
			else
				this.value = arguments[0];
		}
	}

	function setBinary(binary) {
		if (!binary)
			return;

		if ((binary[0] & TLVLength.LONG_FORM) !== TLVLength.LONG_FORM) {
			this.value = binary[0];
		} else {
			var length = 1 + (binary[0] & TLVLength.LONG_FORM_LENGTH_MASK);
			this.value = 0;

			for (var i = 1; i < length; i++)
				this.value = this.value * 256 + binary[i];
		}
	}

	function getBinary() {
		var result = [];

		if (this.value < TLVLength.LONG_FORM) {
			result.push(this.value);
		} else {
			var value = this.value;
			var len = 0;

			while (value > 0) {
				result[len++] = value % 256;
				value = Math.floor(value / 256);
			}

			result[len] = TLVLength.LONG_FORM | len;

			result.reverse();
		}

		return result;
	}

	TLVLength.LONG_FORM = 0x80;
	TLVLength.LONG_FORM_LENGTH_MASK = 0x7F;

	TLVLength.prototype.constructor = TLVLength;

	Object.defineProperties(TLVLength.prototype, {
		binary: {
			set: setBinary,
			get: getBinary,
			enumerable: true,
			configurable: true,
		}
	});

	return TLVLength;
})();

TLVTag = (function() {
	function TLVTag(der) {
		this[0] = der[0];
		this.length = 1;

		if ((der[0] & TLVTag.NUMBER_MASK) === TLVTag.NUMBER_MASK) {
			do {
				this[this.length] = der[this.length];
			} while ((der[this.length++] & TLVTag.CONTINUE) === TLVTag.CONTINUE);
		}
	}

	function getBinary() {
		var binary = [];

		for (var i = 0; i < this.length; i++)
			binary.push(this[i]);

		return binary;
	}

	TLVTag.NUMBER_MASK = 0x1F;
	TLVTag.CONSTRUCTED = 0x20;
	TLVTag.CONTINUE = 0x80;

	TLVTag.prototype.constructor = TLVTag;

	TLVTag.prototype.isConstructed = function() {
		return !!(this[0] & TLVTag.CONSTRUCTED);
	}

	Object.defineProperties(TLVTag.prototype, {
		binary: {
			get: getBinary,
			enumerable: true,
			configurable: true
		}
	});

	return TLVTag;
})();

TLVNode = (function() {
	function TLVNode(input) {
		if (jQuery.isArray(input)) {
			this.binary = input;
		} else if (input instanceof Object) {
			this.jqTreeData = input;
		}
	}

	function toHex(b) {
		var hex= [ '0', '1', '2', '3', '4', '5', '6', '7',
			   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' ];

		return hex[Math.floor(b / 16)] + hex[b % 16];
	}

	function binToHex(bin) {
		return bin.reduce(function(prev, cur, idx, arr) { return prev + toHex(cur); }, "");
	}

	function setBinary(binaryDer) {
		var len, valueSlice, idxValueStart, idxValueEnd;

		if (binaryDer === undefined)
			return;

		this.tag = new TLVTag(binaryDer);
		this.label = binToHex(this.tag.binary);

		len = new TLVLength(binaryDer.slice(this.tag.length));

		idxValueStart = this.tag.length + len.binary.length;
		idxValueEnd = idxValueStart + len.value;
		valueSlice = binaryDer.slice(idxValueStart, idxValueEnd);

		if (this.tag.isConstructed()) {
			delete this.value;
			this.children = [];

			while (idxValueStart < idxValueEnd) {
				var node;

				valueSlice = binaryDer.slice(idxValueStart, idxValueEnd);
				node = new TLVNode(valueSlice);
				this.children.push(node);
				idxValueStart += node.length;
			}
		} else {
			delete this.children;
			this.value = valueSlice;
			this.label += " " + binToHex(this.value);
		}
	}

	function getBinary() {
		var result = this.tag.binary;

		if (this.tag.isConstructed()) {
			var childrenResult = [];

			for (var i = 0; i < this.children.length; i++)
				childrenResult = childrenResult.concat(this.children[i].binary);

			result = result.concat(new TLVLength(childrenResult.length).binary);
			result = result.concat(childrenResult);
		} else {
			result = result.concat(new TLVLength(this.value.length).binary);
			result = result.concat(this.value);
		}

		return result;
	}

	function getJqTreeData() {
		var result = {};

		result.tag = this.tag.binary;
		result.label = binToHex(result.tag);

		if (this.value) {
			result.value = this.value;
			result.label += " " + binToHex(result.value);
		}

		if (this.children)
			result.children = this.children.map(function(cur) { return cur.jqTreeData; });

		return result;
	}

	function setJqTreeData(data) {
		delete this.value;
		delete this.children;

		this.tag = new TLVTag(data.tag);

		if (data.value)
			this.value = data.value;

		if (data.children)
			this.children = data.children.map(function(cur) { return new TLVNode(cur); });
	}

	function setHex(hex) {
		this.binary = TLVDocument.hexToBin(hex);
	}

	function getHex() {
		return TLVDocument.binToHex(this.binary);
	}

	function getLength() {
		var valueLength = 0;

		if (this.tag.isConstructed()) {
			for (var i = 0; i < this.children.length; i++)
				valueLength += this.children[i].length;
		} else {
			valueLength = this.value.length;
		}

		return this.tag.length + new TLVLength(valueLength).binary.length + valueLength;
	}

	TLVNode.prototype.constructor = TLVNode;

	Object.defineProperties(TLVNode.prototype, {
		binary: {
			set: setBinary,
			get: getBinary,
			enumerable: true,
			configurable: true
		},
		hex: {
			set: setHex,
			get: getHex,
			enumerable: true,
			configurable: true
		},
		length: {
			get: getLength,
			enumerable: true,
			configurable: true
		},
		jqTreeData: {
			get: getJqTreeData,
			set: setJqTreeData,
			enumerable: true,
			configurable: true
		}
	});

	return TLVNode;
})();

TLVDocument = (function() {
	function TLVDocument(raw) {
		if (jQuery.isArray(raw)) {
			this.binary = raw;
		} else {
			this.hex = raw;
		}
	}

	function setBinary(binary) {
		var idxValueBegin = 0;

		this.children = [];

		if (!binary)
			return;

		while (idxValueBegin < binary.length) {
			var node = new TLVNode(binary.slice(idxValueBegin));

			idxValueBegin += node.length;
			this.children.push(node);
		}
	}

	function getBinary() {
		return this.children.reduce(function(prev, cur) { return prev.concat(cur.binary) }, []);
	}

	function setHex(hex) {
		this.binary = hexToBin(hex);
	}

	function getHex() {
		return binToHex(this.binary);
	}

	function getLength() {
		return this.children.reduce(function(prev, cur) { return prev + cur.length; }, 0);
	}

	function getJqTreeData() {
		return this.children.map(function(cur) { return cur.jqTreeData; });
	}

	function setJqTreeData(data) {
		this.children = data.children.map(function(cur) {
			var node = new TLVNode();

			node.tag = new TLVTag(cur.tag);

			if (cur.value)
				node.value = cur.value;

			if (cur.children) {
				node.children = cur.children.map(function(cur) {
					var node = new TLVNode();
					node.jqTreeData = cur;
					return node;
				});
			}

			return node;
		});
	}

	TLVDocument.prototype.constructor = TLVDocument;

	Object.defineProperties(TLVDocument.prototype, {
		binary: {
			set: setBinary,
			get: getBinary,
			enumerable: true,
			configurable: true
		},
		hex: {
			set: setHex,
			get: getHex,
			enumerable: true,
			configurable: true
		},
		length: {
			get: getLength,
			enumerable: true,
			configurable: true
		},
		jqTreeData: {
			get: getJqTreeData,
			set: setJqTreeData,
			enumerable: true,
			configurable: true
		}
	});

	function hexToBin(hex) {
		var binary = [];

		if (!hex)
			return binary;

		for (var i = 0; i < hex.length / 2; i++)
			binary[i] = parseInt(hex.substr(i * 2, 2), 16);

		return binary;
	}

	function binToHex(binary) {
		return binary.reduce(function(prev, cur) {
			var hexdigit = [ '0', '1', '2', '3', '4', '5', '6', '7',
			         	 '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' ];

			return prev + hexdigit[Math.floor(cur / 16)] + hexdigit[cur % 16];
		}, "");
	}

	TLVDocument.hexToBin = hexToBin;
	TLVDocument.binToHex = binToHex;

	return TLVDocument;
})();
