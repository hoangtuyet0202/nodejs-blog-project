const mongoose = require('mongoose');
async function connect() {
    try {
        await mongoose.connect(process.env.MONGO_URL || 'mongodb://localhost:27017/f8_edu_dev', {
            useNewUrlParser: true,
            useUnifiedTopology: true,
            useCreateIndex: true,
        });
        console.log('connect Success');
    } catch (error) {
        console.log('connect fail');
    }
}

module.exports = {connect};