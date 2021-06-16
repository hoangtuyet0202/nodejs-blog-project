const mongoose = require('mongoose');
const slug = require('mongoose-slug-generator');
const mongooseDelete = require('mongoose-delete');
const AutoIncrement = require('mongoose-sequence')(mongoose);


const Schema = mongoose.Schema;

// Schema: Lược đồ quan hệ
const CourseSchema = new Schema({
    _id: {type: Number},
    name: [{ type: String, required : true,}],
    description: { type: String},
    image: { type: String},
    videoId : { type: String, required : true,},
    level: { type: String },
    slug: { type: String, slug: 'name', unique : true, }
  }, {
	  _id: false,
    timestamps : true,
  });

CourseSchema.query.sortable = function(req) {
	if(req.query.hasOwnProperty('_sort')) {
		const isValidtype = ['asc','desc'].includes(req.query.type);
		if(!isValidtype) {
			req.query.type = 'desc';
		}
		return this.sort({
			[req.query.column] : req.query.type,
		});
	}
	return this;
}
mongoose.plugin(slug);
CourseSchema.plugin(mongooseDelete,{
  	deletedAt : true,
  	overrideMethods: 'all',});

module.exports = mongoose.model('Course', CourseSchema);